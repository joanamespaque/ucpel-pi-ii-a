#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ping.h"

#define PAYLOAD_SIZE 32
#define PROBE_INTERVAL_MS 1000

/* Result of a single probe: round-trip time in ms, or -1 if lost. */
typedef double (*probe_fn)(void *context, const struct sockaddr *dest, int seq, int timeout_ms);

static int resolve_host(const char *host, struct sockaddr_storage *dest)
{
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(host, NULL, &hints, &result) != 0) {
        return -1;
    }
    memcpy(dest, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
    return 0;
}

static void compute_summary(struct ping_stats *stats)
{
    double total = 0.0;
    int i;

    stats->received = 0;
    stats->min_ms = 0.0;
    stats->max_ms = 0.0;

    for (i = 0; i < stats->sent; i++) {
        double rtt = stats->rtt_ms[i];
        if (rtt < 0) {
            continue;
        }
        if (stats->received == 0 || rtt < stats->min_ms) {
            stats->min_ms = rtt;
        }
        if (stats->received == 0 || rtt > stats->max_ms) {
            stats->max_ms = rtt;
        }
        total += rtt;
        stats->received++;
    }

    stats->avg_ms = stats->received > 0 ? total / stats->received : 0.0;
    stats->loss_percent = stats->sent > 0
        ? (double)(stats->sent - stats->received) * 100.0 / stats->sent
        : 0.0;
}

static void run_probes(struct ping_stats *stats, probe_fn probe, void *context,
                       const struct sockaddr *dest, int count, int timeout_ms)
{
    int seq;

    for (seq = 0; seq < count; seq++) {
        stats->rtt_ms[seq] = probe(context, dest, seq + 1, timeout_ms);
        stats->sent++;
        if (seq < count - 1) {
            platform_sleep_ms(PROBE_INTERVAL_MS);
        }
    }
}

#ifdef _WIN32

#include <iphlpapi.h>
#include <icmpapi.h>

static double probe_icmp_win(void *context, const struct sockaddr *dest, int seq, int timeout_ms)
{
    HANDLE handle = *(HANDLE *)context;
    char payload[PAYLOAD_SIZE];
    union {
        ICMP_ECHO_REPLY v4;
        ICMPV6_ECHO_REPLY v6;
        unsigned char raw[sizeof(ICMP_ECHO_REPLY) + sizeof(ICMPV6_ECHO_REPLY) + PAYLOAD_SIZE + 64];
    } reply;
    double start;
    double elapsed;
    DWORD replies;

    memset(payload, 'a' + (seq % 26), sizeof(payload));
    start = platform_time_ms();

    if (dest->sa_family == AF_INET) {
        IPAddr target = ((const struct sockaddr_in *)dest)->sin_addr.s_addr;
        replies = IcmpSendEcho(handle, target, payload, sizeof(payload), NULL,
                               &reply, sizeof(reply), (DWORD)timeout_ms);
        elapsed = platform_time_ms() - start;
        if (replies == 0 || reply.v4.Status != IP_SUCCESS) {
            return -1.0;
        }
    } else {
        struct sockaddr_in6 source;
        memset(&source, 0, sizeof(source));
        source.sin6_family = AF_INET6;
        replies = Icmp6SendEcho2(handle, NULL, NULL, NULL, &source,
                                 (struct sockaddr_in6 *)dest, payload, sizeof(payload), NULL,
                                 &reply, sizeof(reply), (DWORD)timeout_ms);
        elapsed = platform_time_ms() - start;
        if (replies == 0 || reply.v6.Status != IP_SUCCESS) {
            return -1.0;
        }
    }
    return elapsed;
}

int ping_host(const char *host, int count, int timeout_ms, struct ping_stats *stats)
{
    struct sockaddr_storage dest;
    HANDLE handle;

    memset(stats, 0, sizeof(*stats));
    snprintf(stats->host, sizeof(stats->host), "%s", host);
    if (count > MAX_PROBES) {
        count = MAX_PROBES;
    }
    if (resolve_host(host, &dest) != 0) {
        return PING_ERR_RESOLVE;
    }
    stats->family = dest.ss_family;
    ip_to_string((struct sockaddr *)&dest, stats->address, sizeof(stats->address));

    handle = dest.ss_family == AF_INET ? IcmpCreateFile() : Icmp6CreateFile();
    if (handle == INVALID_HANDLE_VALUE) {
        return PING_ERR_NO_METHOD;
    }
    stats->method = dest.ss_family == AF_INET ? "IcmpSendEcho (IP Helper API)"
                                              : "Icmp6SendEcho2 (IP Helper API)";
    run_probes(stats, probe_icmp_win, &handle, (struct sockaddr *)&dest, count, timeout_ms);
    IcmpCloseHandle(handle);

    compute_summary(stats);
    return PING_OK;
}

#else

#include <errno.h>
#include <poll.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>

struct icmp_socket {
    int fd;
    int raw;
    unsigned short id;
};

struct command_context {
    char host[HOST_LEN];
};

struct echo_header {
    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned short id;
    unsigned short seq;
};

static unsigned short icmp_checksum(const void *data, size_t length)
{
    const unsigned short *words = (const unsigned short *)data;
    unsigned long sum = 0;

    while (length > 1) {
        sum += *words++;
        length -= 2;
    }
    if (length == 1) {
        sum += *(const unsigned char *)words;
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return (unsigned short)~sum;
}

/*
 * Unprivileged ICMP datagram sockets depend on net.ipv4.ping_group_range;
 * raw sockets need root or CAP_NET_RAW. Returns -1 when neither is allowed.
 */
static int open_icmp_socket(int family, struct icmp_socket *sock)
{
    int protocol = family == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6;

    sock->id = (unsigned short)(getpid() & 0xffff);
    sock->raw = 0;
    sock->fd = socket(family, SOCK_DGRAM, protocol);
    if (sock->fd < 0) {
        sock->raw = 1;
        sock->fd = socket(family, SOCK_RAW, protocol);
    }
    return sock->fd < 0 ? -1 : 0;
}

/* Returns 1 when the received packet is the echo reply for seq. */
static int is_matching_reply(const struct icmp_socket *sock, int family,
                             const unsigned char *packet, ssize_t length, int seq)
{
    struct echo_header reply;
    unsigned char expected_type = family == AF_INET ? ICMP_ECHOREPLY : ICMP6_ECHO_REPLY;

    /* Raw IPv4 sockets deliver the IP header before the ICMP message. */
    if (sock->raw && family == AF_INET) {
        size_t header_length = (size_t)(packet[0] & 0x0f) * 4;
        if (length < (ssize_t)header_length) {
            return 0;
        }
        packet += header_length;
        length -= (ssize_t)header_length;
    }
    if (length < (ssize_t)sizeof(struct echo_header)) {
        return 0;
    }

    memcpy(&reply, packet, sizeof(reply));
    if (reply.type != expected_type || ntohs(reply.seq) != seq) {
        return 0;
    }
    /* Datagram sockets have their identifier rewritten by the kernel. */
    return !sock->raw || ntohs(reply.id) == sock->id;
}

static double probe_icmp_socket(void *context, const struct sockaddr *dest, int seq, int timeout_ms)
{
    struct icmp_socket *sock = (struct icmp_socket *)context;
    struct {
        struct echo_header header;
        unsigned char payload[PAYLOAD_SIZE];
    } packet;
    unsigned char buffer[1500];
    socklen_t dest_length = dest->sa_family == AF_INET ? sizeof(struct sockaddr_in)
                                                       : sizeof(struct sockaddr_in6);
    double start;
    double elapsed;

    memset(&packet, 0, sizeof(packet));
    memset(packet.payload, 'a' + (seq % 26), PAYLOAD_SIZE);
    packet.header.type = dest->sa_family == AF_INET ? ICMP_ECHO : ICMP6_ECHO_REQUEST;
    packet.header.id = htons(sock->id);
    packet.header.seq = htons((unsigned short)seq);
    /* The kernel computes the ICMPv6 checksum itself; ICMPv4 needs it here. */
    if (dest->sa_family == AF_INET) {
        packet.header.checksum = icmp_checksum(&packet, sizeof(packet));
    }

    start = platform_time_ms();
    if (sendto(sock->fd, &packet, sizeof(packet), 0, dest, dest_length) < 0) {
        return -1.0;
    }

    elapsed = 0.0;
    while (elapsed < timeout_ms) {
        struct pollfd pfd;
        ssize_t received;

        pfd.fd = sock->fd;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, (int)(timeout_ms - elapsed)) <= 0) {
            return -1.0;
        }
        received = recv(sock->fd, buffer, sizeof(buffer), 0);
        elapsed = platform_time_ms() - start;
        if (received > 0 && is_matching_reply(sock, dest->sa_family, buffer, received, seq)) {
            return elapsed;
        }
    }
    return -1.0;
}

/* Last resort: runs the system ping command once and parses "time=<ms>". */
static double probe_ping_command(void *context, const struct sockaddr *dest, int seq, int timeout_ms)
{
    struct command_context *cmd = (struct command_context *)context;
    char command[HOST_LEN + 64];
    char line[512];
    double rtt = -1.0;
    int timeout_s = timeout_ms < 1000 ? 1 : timeout_ms / 1000;
    FILE *output;

    (void)seq;
    snprintf(command, sizeof(command), "ping %s -c 1 -W %d %s 2>/dev/null",
             dest->sa_family == AF_INET6 ? "-6" : "-4", timeout_s, cmd->host);

    output = popen(command, "r");
    if (output == NULL) {
        return -1.0;
    }
    while (fgets(line, sizeof(line), output) != NULL) {
        char *time_field = strstr(line, "time=");
        if (time_field != NULL) {
            rtt = atof(time_field + 5);
        }
    }
    pclose(output);
    return rtt;
}

int ping_host(const char *host, int count, int timeout_ms, struct ping_stats *stats)
{
    struct sockaddr_storage dest;
    struct icmp_socket sock;

    memset(stats, 0, sizeof(*stats));
    snprintf(stats->host, sizeof(stats->host), "%s", host);
    if (count > MAX_PROBES) {
        count = MAX_PROBES;
    }
    if (resolve_host(host, &dest) != 0) {
        return PING_ERR_RESOLVE;
    }
    stats->family = dest.ss_family;
    ip_to_string((struct sockaddr *)&dest, stats->address, sizeof(stats->address));

    if (open_icmp_socket(dest.ss_family, &sock) == 0) {
        stats->method = sock.raw ? "ICMP raw socket" : "ICMP datagram socket";
        run_probes(stats, probe_icmp_socket, &sock, (struct sockaddr *)&dest, count, timeout_ms);
        close(sock.fd);
    } else if (system("ping -V > /dev/null 2>&1") == 0) {
        struct command_context cmd;
        /* The resolved address is passed to avoid shell injection through the host name. */
        snprintf(cmd.host, sizeof(cmd.host), "%s", stats->address);
        stats->method = "system ping command";
        run_probes(stats, probe_ping_command, &cmd, (struct sockaddr *)&dest, count, timeout_ms);
    } else {
        return PING_ERR_NO_METHOD;
    }

    compute_summary(stats);
    return PING_OK;
}

#endif
