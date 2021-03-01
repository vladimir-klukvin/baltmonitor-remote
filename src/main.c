/**
 * @file main.c
 * @author V.K.
 * @brief
 * @date 2021-03-01
 *
 * @copyright Copyright Balt-System Ltd. <info@bsystem.ru>
 * All rights reserved.
 *
 */
#include <getopt.h>
#include <malloc.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "log.h"
#include "server.h"

void on_sigint(int32_t _)
{
    log_info("Exit");
    server_stop();
    exit(EXIT_SUCCESS);
}

void configure_logging(void)
{
    log_reset_state();
    /* TODO: Select output by arg */
    log_set_out_stdout();

#ifdef NDEBUG
    log_set_log_level(LOG_MAX_LEVEL_ERROR_WARNING_INFO);
#else
    log_set_log_level(LOG_MAX_LEVEL_ERROR_WARNING_INFO_DEBUG);
#endif
}

void usage(void)
{
    /* clang-format off */
    printf("Usage:\n");
    printf("  %s [[-a <ip_address>] [-p <port_num>] [-n <count>]] | [-h] \n",PROGRAM_NAME);
    printf("\n");
    printf("Options:\n");
    printf("  -a, --address <ip_address>     start server at <ip_address> ip-address\n");
    printf("  -p, --port <port_num>          server will use <port_num> port\n");
    printf("  -n, --nconnections <count>     can serve simultaneously <count> clients\n");
    printf("  -h, --help                     give this help list\n");
    printf("\n");
    printf("Mandatory or optional arguments to long options are also mandatory or optional\n");
    printf("for any corresponding short options.\n");
    /* clang-format on */
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

    configure_logging();

    server_start(addr, port, max_connections);

    exit(EXIT_FAILURE);
}
