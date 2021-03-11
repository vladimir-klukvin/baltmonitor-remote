/**
 * @file main.c
 * @author V.K.
 * @brief
 * @date 2021-03-01
 *
 * @copyright Copyright (c) 2021 Balt-System Ltd. <info@bsystem.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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

enum log_location {
    LOG_LOCATION_STDOUT,
    LOG_LOCATION_FILE,
    LOG_LOCATION_SYSLOG
};

void on_sigint(int32_t _)
{
    server_stop();
    exit(EXIT_SUCCESS);
}

void configure_logging(enum log_location loc, const char_t *file)
{
    log_reset_state();

    switch (loc) {
    case LOG_LOCATION_STDOUT:
        log_set_out_stdout();
        break;
    case LOG_LOCATION_FILE:
        log_set_log_file(file);
        break;
    case LOG_LOCATION_SYSLOG:
    default:
        break;
    }

#ifdef NDEBUG
    log_set_min_level(LOG_LEVEL_INFO);
#else
    log_set_min_level(LOG_LEVEL_DEBUG);
#endif
}

void usage(void)
{
    /* clang-format off */
    printf("Usage:\n");
    printf("  %s [[-a IP_ADDRESS] [-p PORT_NUM] [-m COUNT] [[-f[=FILE_NAME]] | [-s]]] | [-h] \n", PROGRAM_NAME);
    printf("\n");
    printf("Options:\n");
    printf("  -a, --address=IP_ADDRESS       start server at IP_ADDRESS \n");
    printf("  -p, --port=PORT_NUM            server will listen PORT_NUM\n");
    printf("  -m, --max-clients=COUNT        can serve simultaneously COUNT clients\n");
    printf("  -f, --file[=FILE_NAME]         server logs will be stored in the FILE_NAME,\n");
    printf("                                 default: server.log\n");
    printf("  -s, --syslog                   server logs will be stored in the system log\n");
    printf("  -h, --help                     give this help list\n");
    printf("\n");
    printf("By default server put logs into stdout. Use --file[=FILE_NAME] or --syslog to\n");
    printf("store logs in another location.\n");
    printf("\n");
    printf("Mandatory or optional arguments to long options are also mandatory or optional\n");
    printf("for any corresponding short options.\n");
    /* clang-format on */
}

int32_t main(int32_t argc, char_t *argv[])
{
    char_t *addr = malloc(10);
    strcpy(addr, "127.0.0.1");
    int32_t port = 65000;
    int32_t max_clients = 50;
    char_t *log_file = malloc(11);
    strcpy(log_file, "server.log");
    enum log_location log_loc = LOG_LOCATION_STDOUT;

    static struct option long_options[] = {
        {"address", required_argument, 0, 'a'},
        {"port", required_argument, 0, 'p'},
        {"max-clients", required_argument, 0, 'm'},
        {"file", optional_argument, 0, 'f'},
        {"syslog", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    while (true) {
        int c = getopt_long(argc, argv, "a:p:m:f::sh", long_options, NULL);
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
        case 'm':
            max_clients = atoi(optarg);
        case 'f':
            if (log_loc != LOG_LOCATION_STDOUT) {
                printf("Invalid option: %s\n", argv[optind]);
                break;
            }

            if (optarg != NULL) {
                strcpy(log_file, optarg);
            }
            log_loc = LOG_LOCATION_FILE;
            break;
        case 's':
            if (log_loc != LOG_LOCATION_STDOUT) {
                printf("Invalid option: %s\n", argv[optind]);
                break;
            }
            log_loc = LOG_LOCATION_SYSLOG;
            break;
        case 'h':
        default: /* '?' */
            usage();
            exit(EXIT_SUCCESS);
        }
    }

    if (optind < argc) {
        while (optind < argc) {
            printf("invalid option -- '%s'\n", argv[optind++]);
        }
        printf("\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, on_sigint);
    atexit(server_stop);

    configure_logging(log_loc, log_file);

    server_start(addr, (uint16_t)port, max_clients);
}
