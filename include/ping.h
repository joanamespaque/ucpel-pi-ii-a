#ifndef PING_H
#define PING_H

#include "ip_utils.h"

#define MAX_PROBES 100
#define HOST_LEN 256

#define PING_OK 0
#define PING_ERR_RESOLVE -1
#define PING_ERR_NO_METHOD -2

struct ping_stats {
    char host[HOST_LEN];
    char address[IP_STR_LEN];
    int family;
    int sent;
    int received;
    double rtt_ms[MAX_PROBES]; /* negative when the probe was lost */
    double min_ms;
    double avg_ms;
    double max_ms;
    double loss_percent;
    const char *method;
};

/*
 * Sends count ICMP echo requests to host (name or IPv4/IPv6 literal) and
 * computes latency and packet loss. Returns PING_OK, PING_ERR_RESOLVE when the
 * host cannot be resolved, or PING_ERR_NO_METHOD when ICMP cannot be used.
 */
int ping_host(const char *host, int count, int timeout_ms, struct ping_stats *stats);

#endif
