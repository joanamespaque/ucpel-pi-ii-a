#ifndef REPORT_H
#define REPORT_H

#include "interfaces.h"
#include "ping.h"

void print_header(void);
void print_interfaces(const struct ip_entry *entries, int count);
void print_ping_stats(const struct ping_stats *stats);
void print_ping_error(int error, const char *host);

#endif
