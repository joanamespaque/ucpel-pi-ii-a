#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interfaces.h"

static void fill_entry(struct ip_entry *entry, const char *name,
                       const struct sockaddr *addr, int prefix_length)
{
    snprintf(entry->interface_name, sizeof(entry->interface_name), "%s", name);
    entry->family = addr->sa_family;
    entry->prefix_length = prefix_length;
    entry->scope = ip_scope_name(addr);
    entry->ipv4_class = ipv4_class(addr);
    if (ip_to_string(addr, entry->address, sizeof(entry->address)) != 0) {
        snprintf(entry->address, sizeof(entry->address), "?");
    }
}

#ifdef _WIN32

#include <iphlpapi.h>

int collect_interfaces(struct ip_entry *entries, int max)
{
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    ULONG size = 16 * 1024;
    IP_ADAPTER_ADDRESSES *adapters = NULL;
    IP_ADAPTER_ADDRESSES *adapter;
    ULONG result;
    int attempts = 0;
    int count = 0;

    /* The required buffer size can change between calls, so retry a few times. */
    do {
        free(adapters);
        adapters = (IP_ADAPTER_ADDRESSES *)malloc(size);
        if (adapters == NULL) {
            return -1;
        }
        result = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, adapters, &size);
        attempts++;
    } while (result == ERROR_BUFFER_OVERFLOW && attempts < 3);

    if (result != NO_ERROR) {
        free(adapters);
        return -1;
    }

    for (adapter = adapters; adapter != NULL; adapter = adapter->Next) {
        IP_ADAPTER_UNICAST_ADDRESS *unicast;
        char name[IFACE_NAME_LEN];

        if (adapter->OperStatus != IfOperStatusUp) {
            continue;
        }
        if (WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1,
                                name, sizeof(name), NULL, NULL) == 0) {
            snprintf(name, sizeof(name), "%s", adapter->AdapterName);
        }

        for (unicast = adapter->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next) {
            if (count >= max) {
                break;
            }
            fill_entry(&entries[count], name, unicast->Address.lpSockaddr,
                       (int)unicast->OnLinkPrefixLength);
            count++;
        }
    }

    free(adapters);
    return count;
}

#else

#include <ifaddrs.h>
#include <net/if.h>

/* Counts the bits set in a netmask to obtain the CIDR prefix length. */
static int netmask_prefix(const struct sockaddr *mask)
{
    const unsigned char *bytes;
    int length;
    int prefix = 0;
    int i;

    if (mask == NULL) {
        return -1;
    }
    if (mask->sa_family == AF_INET) {
        bytes = (const unsigned char *)&((const struct sockaddr_in *)mask)->sin_addr;
        length = 4;
    } else if (mask->sa_family == AF_INET6) {
        bytes = ((const struct sockaddr_in6 *)mask)->sin6_addr.s6_addr;
        length = 16;
    } else {
        return -1;
    }

    for (i = 0; i < length; i++) {
        unsigned char byte = bytes[i];
        while (byte != 0) {
            prefix += byte & 1;
            byte >>= 1;
        }
    }
    return prefix;
}

int collect_interfaces(struct ip_entry *entries, int max)
{
    struct ifaddrs *list;
    struct ifaddrs *item;
    int count = 0;

    if (getifaddrs(&list) != 0) {
        return -1;
    }

    for (item = list; item != NULL && count < max; item = item->ifa_next) {
        if (item->ifa_addr == NULL || !(item->ifa_flags & IFF_UP)) {
            continue;
        }
        if (item->ifa_addr->sa_family != AF_INET && item->ifa_addr->sa_family != AF_INET6) {
            continue;
        }
        fill_entry(&entries[count], item->ifa_name, item->ifa_addr, netmask_prefix(item->ifa_netmask));
        count++;
    }

    freeifaddrs(list);
    return count;
}

#endif
