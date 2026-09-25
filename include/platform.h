#ifndef PLATFORM_H
#define PLATFORM_H

/*
 * Single entry point for OS-specific headers, so the other modules can use
 * sockets and address structures without repeating #ifdef blocks.
 */
#ifdef _WIN32
    /* inet_ntop and the ICMPv6 helper APIs require Windows Vista or later. */
    #if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600
        #undef _WIN32_WINNT
        #define _WIN32_WINNT 0x0600
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #define PLATFORM_NAME "Windows"
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #define PLATFORM_NAME "Linux"
#endif

int platform_init(void);
void platform_cleanup(void);
void platform_sleep_ms(unsigned int ms);
double platform_time_ms(void);

#endif
