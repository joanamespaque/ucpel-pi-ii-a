#ifndef IP_UTILS_H
#define IP_UTILS_H

#include <stddef.h>
#include "platform.h"

#define IP_STR_LEN 64

const char *ip_type_name(int family);
int ip_to_string(const struct sockaddr *addr, char *buffer, size_t size);
const char *ip_scope_name(const struct sockaddr *addr);
char ipv4_class(const struct sockaddr *addr);

#endif
