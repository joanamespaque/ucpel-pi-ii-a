#include <stdio.h>
#include "report.h"

#define LINE "------------------------------------------------------------------------------"

static const char *latency_rating(double avg_ms)
{
    if (avg_ms < 50.0) {
        return "Excellent";
    }
    if (avg_ms < 100.0) {
        return "Good";
    }
    if (avg_ms < 200.0) {
        return "Fair";
    }
    return "Poor";
}

static const char *loss_rating(double loss_percent)
{
    if (loss_percent == 0.0) {
        return "No loss";
    }
    if (loss_percent <= 5.0) {
        return "Acceptable";
    }
    if (loss_percent < 100.0) {
        return "Unstable connection";
    }
    return "Host unreachable";
}

void print_header(void)
{
    printf("\n==============================================================================\n");
    printf("                               Monitor de Rede\n");
    printf("==============================================================================\n");
    printf("Platform: %s\n", PLATFORM_NAME);
}

void print_interfaces(const struct ip_entry *entries, int count)
{
    int ipv4_count = 0;
    int ipv6_count = 0;
    int i;

    printf("\n[IP addresses]\n%s\n", LINE);
    printf("%-18s %-5s %-40s %-13s %s\n", "Interface", "Type", "Address/Prefix", "Scope", "Class");
    printf("%s\n", LINE);

    for (i = 0; i < count; i++) {
        char address[IP_STR_LEN + 8];

        if (entries[i].prefix_length >= 0) {
            snprintf(address, sizeof(address), "%s/%d", entries[i].address, entries[i].prefix_length);
        } else {
            snprintf(address, sizeof(address), "%s", entries[i].address);
        }

        printf("%-18.18s %-5s %-40s %-13s %c\n", entries[i].interface_name,
               ip_type_name(entries[i].family), address, entries[i].scope, entries[i].ipv4_class);

        if (entries[i].family == AF_INET) {
            ipv4_count++;
        } else if (entries[i].family == AF_INET6) {
            ipv6_count++;
        }
    }

    printf("%s\n", LINE);
    printf("Total: %d address(es) | IPv4: %d | IPv6: %d\n", count, ipv4_count, ipv6_count);
}

void print_ping_stats(const struct ping_stats *stats)
{
    int i;

    printf("\n[Latency and packet loss]\n%s\n", LINE);
    printf("Target : %s (%s, %s)\n", stats->host, stats->address, ip_type_name(stats->family));
    printf("Method : %s\n", stats->method);
    printf("%s\n", LINE);

    for (i = 0; i < stats->sent; i++) {
        if (stats->rtt_ms[i] >= 0) {
            printf("Reply from %s: seq=%d time=%.3f ms\n", stats->address, i + 1, stats->rtt_ms[i]);
        } else {
            printf("Request seq=%d timed out\n", i + 1);
        }
    }

    printf("%s\n", LINE);
    printf("Packets: sent=%d received=%d lost=%d\n",
           stats->sent, stats->received, stats->sent - stats->received);
    printf("Packet loss rate: %.1f%% (%s)\n", stats->loss_percent, loss_rating(stats->loss_percent));

    if (stats->received > 0) {
        printf("Latency: min=%.3f ms avg=%.3f ms max=%.3f ms (%s)\n",
               stats->min_ms, stats->avg_ms, stats->max_ms, latency_rating(stats->avg_ms));
    } else {
        printf("Latency: unavailable (no replies received)\n");
    }
}

void print_ping_error(int error, const char *host)
{
    if (error == PING_ERR_RESOLVE) {
        printf("\nError: could not resolve host \"%s\".\n", host);
    } else if (error == PING_ERR_NO_METHOD) {
        printf("\nError: ICMP is not available. Run as administrator/root or enable "
               "unprivileged ICMP (sysctl net.ipv4.ping_group_range).\n");
    }
}
