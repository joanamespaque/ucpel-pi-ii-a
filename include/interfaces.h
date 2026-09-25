#ifndef INTERFACES_H
#define INTERFACES_H

#include "ip_utils.h"

#define MAX_INTERFACES 64
#define IFACE_NAME_LEN 128

struct ip_entry {
    char interface_name[IFACE_NAME_LEN];
    int family;
    char address[IP_STR_LEN];
    int prefix_length;
    const char *scope;
    char ipv4_class;
};

/* Fills up to max entries and returns how many were found, or -1 on error. */
int collect_interfaces(struct ip_entry *entries, int max);

#endif
