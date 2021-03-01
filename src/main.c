#include <stdlib.h>
#include <getopt.h>
#include <malloc.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "global.h"
#include "server.h"

void on_sigint(int32_t _)
{
    printf("\nExit\n");
    server_stop();
    exit(EXIT_SUCCESS);
}

void usage(void)
{
}

int32_t main(int32_t argc, char_t *argv[])
{
    signal(SIGINT, on_sigint);
    char_t *addr = malloc(10);
    strcpy(addr, "127.0.0.1");
    int32_t port = 65000;
    int32_t max_connections = 50;

    static struct option long_options[] = {
        {"address", required_argument, 0, 'a'},
        {"port", required_argument, 0, 'p'},
        {"nconnections", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    while (true) {
        int c = getopt_long(argc, argv, "a:p:n:h", long_options, NULL);
        if (c == -1)
            break;

        switch (c) {
        case 'a':
            addr = realloc(addr, strlen(optarg) + 1);
            strcpy(addr, optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'n':
            max_connections = atoi(optarg);
        case 'h':
            usage();
            return 0;

        default: /* '?' */
            usage();
            return 0;
        }
    }

    if (optind < argc) {
        while (optind < argc)
            printf("Invalid option: %s \n", argv[optind++]);
        printf("\n");
    }
    
    server_start(addr, port, max_connections);

    exit(EXIT_FAILURE);
}
