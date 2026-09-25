#include <stdio.h>
#include "menu.h"
#include "report.h"

enum menu_option {
    OPTION_EXIT = 0,
    OPTION_ADDRESSES = 1,
    OPTION_LATENCY = 2,
    OPTION_FULL_REPORT = 3
};

/* Discards the rest of the input line after an invalid scanf read. */
static void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

/* Returns 1 on success or 0 when the input stream has ended. */
static int read_int(const char *prompt, int min, int max, int *value)
{
    int result;

    while (1) {
        printf("%s", prompt);
        result = scanf("%d", value);
        if (result == EOF) {
            return 0;
        }
        clear_input();
        if (result == 1 && *value >= min && *value <= max) {
            return 1;
        }
        printf("Invalid value. Enter a number between %d and %d.\n", min, max);
    }
}

static void show_addresses(void)
{
    struct ip_entry entries[MAX_INTERFACES];
    int count = collect_interfaces(entries, MAX_INTERFACES);

    if (count < 0) {
        printf("\nError: could not read the network interfaces.\n");
        return;
    }
    print_interfaces(entries, count);
}

static void show_latency(const char *host, int probes)
{
    struct ping_stats stats;
    int result;

    printf("\nSending %d ICMP echo request(s) to %s...\n", probes, host);
    result = ping_host(host, probes, PROBE_TIMEOUT_MS, &stats);
    if (result != PING_OK) {
        print_ping_error(result, host);
        return;
    }
    print_ping_stats(&stats);
}

static int ask_latency_target(void)
{
    char host[HOST_LEN];
    int probes;

    printf("Host or IP address (e.g. %s, google.com, ::1): ", DEFAULT_HOST);
    if (scanf("%255s", host) != 1) {
        return 0;
    }
    clear_input();
    if (!read_int("Number of packets (1-100): ", 1, MAX_PROBES, &probes)) {
        return 0;
    }
    show_latency(host, probes);
    return 1;
}

static void print_menu(void)
{
    printf("\n%d - List IP addresses and IP type\n", OPTION_ADDRESSES);
    printf("%d - Measure latency and packet loss\n", OPTION_LATENCY);
    printf("%d - Full report (addresses + latency to %s)\n", OPTION_FULL_REPORT, DEFAULT_HOST);
    printf("%d - Exit\n", OPTION_EXIT);
}

void run_menu(void)
{
    int option = -1;

    while (option != OPTION_EXIT) {
        print_menu();
        if (!read_int("Choose an option: ", OPTION_EXIT, OPTION_FULL_REPORT, &option)) {
            break;
        }

        switch (option) {
        case OPTION_ADDRESSES:
            show_addresses();
            break;
        case OPTION_LATENCY:
            if (!ask_latency_target()) {
                option = OPTION_EXIT;
            }
            break;
        case OPTION_FULL_REPORT:
            show_addresses();
            show_latency(DEFAULT_HOST, DEFAULT_PROBES);
            break;
        default:
            break;
        }
    }

    printf("\nExiting the network monitor.\n");
}
