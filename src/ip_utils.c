#include <string.h>
#include "ip_utils.h"

const char *ip_type_name(int family)
{
    if (family == AF_INET) {
        return "IPv4";
    }
    if (family == AF_INET6) {
        return "IPv6";
    }
    return "Unknown";
}

int ip_to_string(const struct sockaddr *addr, char *buffer, size_t size)
{
    const void *raw;

    if (addr->sa_family == AF_INET) {
        raw = &((const struct sockaddr_in *)addr)->sin_addr;
    } else if (addr->sa_family == AF_INET6) {
        raw = &((const struct sockaddr_in6 *)addr)->sin6_addr;
    } else {
        return -1;
    }

    return inet_ntop(addr->sa_family, raw, buffer, (socklen_t)size) != NULL ? 0 : -1;
}

/* Copies the address bytes in network order, so octets[0] is the leftmost octet. */
static void ipv4_octets(const struct sockaddr *addr, unsigned char octets[4])
{
    memcpy(octets, &((const struct sockaddr_in *)addr)->sin_addr, 4);
}

static const char *ipv4_scope(const struct sockaddr *addr)
{
    unsigned char o[4];
    ipv4_octets(addr, o);

    if (o[0] == 127) {
        return "Loopback";
    }
    if (o[0] == 10 || (o[0] == 172 && o[1] >= 16 && o[1] <= 31) || (o[0] == 192 && o[1] == 168)) {
        return "Private";
    }
    if (o[0] == 169 && o[1] == 254) {
        return "Link-local";
    }
    if (o[0] >= 224 && o[0] <= 239) {
        return "Multicast";
    }
    if (o[0] == 0) {
        return "Unspecified";
    }
    return "Public";
}

static const char *ipv6_scope(const struct sockaddr *addr)
{
    static const unsigned char loopback[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    static const unsigned char unspecified[16] = { 0 };
    const unsigned char *b = ((const struct sockaddr_in6 *)addr)->sin6_addr.s6_addr;

    if (memcmp(b, loopback, 16) == 0) {
        return "Loopback";
    }
    if (memcmp(b, unspecified, 16) == 0) {
        return "Unspecified";
    }
    if (b[0] == 0xfe && (b[1] & 0xc0) == 0x80) {
        return "Link-local";
    }
    if ((b[0] & 0xfe) == 0xfc) {
        return "Unique local";
    }
    if (b[0] == 0xff) {
        return "Multicast";
    }
    return "Global";
}

const char *ip_scope_name(const struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET) {
        return ipv4_scope(addr);
    }
    if (addr->sa_family == AF_INET6) {
        return ipv6_scope(addr);
    }
    return "Unknown";
}

/* Legacy classful addressing (RFC 791); returns '-' for non-IPv4 addresses. */
char ipv4_class(const struct sockaddr *addr)
{
    unsigned char o[4];

    if (addr->sa_family != AF_INET) {
        return '-';
    }

    ipv4_octets(addr, o);
    if (o[0] < 128) {
        return 'A';
    }
    if (o[0] < 192) {
        return 'B';
    }
    if (o[0] < 224) {
        return 'C';
    }
    if (o[0] < 240) {
        return 'D';
    }
    return 'E';
}
