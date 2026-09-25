#include <stdio.h>
#include "platform.h"
#include "report.h"
#include "menu.h"

int main(void)
{
    if (platform_init() != 0) {
        fprintf(stderr, "Error: failed to initialize the network subsystem.\n");
        return 1;
    }

    /* Keeps prompts visible when output is piped or redirected. */
    setvbuf(stdout, NULL, _IONBF, 0);

    print_header();
    run_menu();

    platform_cleanup();
    return 0;
}
