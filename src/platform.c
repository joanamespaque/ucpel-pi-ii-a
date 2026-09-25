#include "platform.h"

#ifdef _WIN32

int platform_init(void)
{
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0 ? 0 : -1;
}

void platform_cleanup(void)
{
    WSACleanup();
}

void platform_sleep_ms(unsigned int ms)
{
    Sleep(ms);
}

double platform_time_ms(void)
{
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / (double)frequency.QuadPart;
}

#else

#include <time.h>

int platform_init(void)
{
    return 0;
}

void platform_cleanup(void)
{
}

void platform_sleep_ms(unsigned int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

double platform_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

#endif
