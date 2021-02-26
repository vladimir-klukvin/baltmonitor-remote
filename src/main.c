#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <malloc.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "global.h"

#define SERVER_PORT        65000
#define SOCKET_BUFFER_SIZE 20000

enum response_type {
    RESPONCE_MAKE_SESSION_SUCCESS,
    RESPONCE_MAKE_SESSION_FAIL,
    RESPONSE_JOIN_SESSION_SUCCESS,
    RESPONSE_JOIN_SESSION_FAIL,
    RESPONSE_SESSION_CLOSED_BY_HOST,
    RESPONSE_SESSION_CLOSED_BY_TARGET,
    RESPONSE_DATA = 'D',
};

enum request_type {
    REQUEST_MAKE_SESSION = 'M',
    REQUEST_JOIN_SESSION = 'J',
    REQUEST_CLOSE_SESSION = 'C',
    REQUEST_DATA = 'D',
};

enum role { ROLE_HOST = 'H', ROLE_TARGET = 'T' };

struct response {
    struct response_header {
        enum response_type resp_type;
        uint16_t session_id;
        size_t body_size;
    } header;
    uint8_t body[];
};

struct request {
    struct request_header {
        enum request_type req_type : 8;
        enum role role : 8;
        uint16_t session_id;
        size_t body_size;
    } header;
    uint8_t body[];
};

int32_t server_sockfd;
pthread_t threads[60];
int32_t num_of_threads = 0;

int32_t opened_sockets[60];
int32_t num_of_opened_sockets = 0;

void join_threads(void)
{
    for (int8_t i = 0; i < num_of_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    num_of_threads = 0;
}

void close_sockets(void)
{
    for (int8_t i = 0; i < num_of_opened_sockets; i++) {
        shutdown(opened_sockets[i], SHUT_RDWR);
        close(opened_sockets[i]);
    }

    num_of_opened_sockets = 0;
}

void on_sigint(int32_t _)
{
    printf("\nExit\n");
    close_sockets();
    close(server_sockfd);
    shutdown(server_sockfd, SHUT_RDWR);
    join_threads();
    exit(EXIT_SUCCESS);
}

/* TODO:
 * pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
 * pthread_mutex_lock(&lock);
 * pthread_mutex_unlock(&lock);
 */

struct session_info {
    uint16_t session_id;
    bool is_host_connected;
    bool is_target_connected;
    int32_t host_sockfd;
    int32_t target_sockfd;
};

struct session_info test_info;

void close_host_connection(struct session_info *session)
{
    session->is_host_connected = false;

    close(session->host_sockfd);

    if (session->is_target_connected) {
        struct response resp;
        resp.header.body_size = 0;
        resp.header.resp_type = RESPONSE_SESSION_CLOSED_BY_HOST;
        resp.header.session_id = session->session_id;
        send(session->target_sockfd, &resp, sizeof(resp), 0);
    }
}

void close_target_connection(struct session_info *session)
{
    session->is_target_connected = false;

    close(session->host_sockfd);

    if (session->is_host_connected) {
        struct response resp;
        resp.header.body_size = 0;
        resp.header.resp_type = RESPONSE_SESSION_CLOSED_BY_TARGET;
        resp.header.session_id = session->session_id;
        send(session->host_sockfd, &resp, sizeof(resp), 0);
    }
}

void host_routine(struct session_info *session)
{
    char_t *buffer = malloc(SOCKET_BUFFER_SIZE);

    session->is_host_connected = true;

    while (true) {
        ssize_t req_size =
            recv(session->host_sockfd, buffer, sizeof(buffer), 0);

        if (req_size <= 0) {
            close_host_connection(session);
            break;
        }

        if (memcmp(buffer, "exit\r\n", 6) == 0) {
            close_host_connection(session);
            break;
        }

        if (session->is_target_connected) {
            send(session->target_sockfd, buffer, req_size, 0);
        }
    }
    free(buffer);
}

void target_routine(struct session_info *session)
{
    char_t *buffer = malloc(SOCKET_BUFFER_SIZE);

    session->is_target_connected = true;

    while (true) {
        size_t req_size =
            recv(session->target_sockfd, buffer, sizeof(buffer), 0);

        if (req_size <= 0) {
            close_target_connection(session);
            break;
        }

        if (memcmp(buffer, "exit\r\n", 6) == 0) {
            close_target_connection(session);
            break;
        }

        if (session->is_host_connected) {
            send(session->host_sockfd, buffer, req_size, 0);
        }
    }
    free(buffer);
}

void *socket_thread(void *arg)
{
    int32_t sockfd = *((int32_t *)arg);

    char_t buffer[1000];
    recv(sockfd, buffer, sizeof(buffer), 0);

    struct request *req = (struct request *)buffer;
    enum role role;
    if (req->header.req_type == REQUEST_MAKE_SESSION) {
        role = ROLE_HOST;
        test_info.session_id = 'AB';
        test_info.is_host_connected = true;
        test_info.host_sockfd = sockfd;

        send(sockfd, &test_info.session_id, sizeof(test_info.session_id), 0);
    } else if (req->header.req_type == REQUEST_JOIN_SESSION) {
        /* TODO: find session */
        role = ROLE_TARGET;
        test_info.is_target_connected = true;
        test_info.target_sockfd = sockfd;
    }

    if (role == ROLE_HOST) {
        host_routine(&test_info);
    } else {
        target_routine(&test_info);
    }

    /* TODO: if both not connected remove session */

    printf("Exit socket_thread\n");
    close(sockfd);
    pthread_exit(NULL);
}

void Usage(void)
{
}

int32_t main(int32_t argc, char_t *argv[])
{
    signal(SIGINT, on_sigint);
    char *addr = malloc(10);
    strcpy(addr, "127.0.0.1");
    int32_t port;
    int32_t max_connections;

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
            printf("%s",addr);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'n':
            max_connections = atoi(optarg);
        case 'h':
            Usage();
            return 0;

        default: /* '?' */
            Usage();
            return 0;
        }
    }

    if (optind < argc) {
        while (optind < argc)
            printf("Invalid option: %s \n", argv[optind++]);
        printf("\n");
    }

    /*
     * Create the server socket
     * IP protocol family
     * Sequenced, reliable, connection-based byte streams
     * Auto-chosen protocol
     */
    server_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int32_t one = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int32_t));

    /* Server socket address */
    struct sockaddr_in server_addr;
    /*
     * Configure settings of the server address struct
     * Address family: Internet
     */
    server_addr.sin_family = AF_INET;

    /* Set port number, using htons function to use proper byte order */
    server_addr.sin_port = htons(SERVER_PORT);

    /* Set IP address */
    server_addr.sin_addr.s_addr = inet_addr(addr);

    /* Set all bits of the padding field to 0 */
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    /* Bind the address struct to the socket */
    if (bind(server_sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        printf("Unable to bind\n");
        exit(1);
    }

    /* Listen on the socket, with 50 max connection requests queued */
    if (listen(server_sockfd, 50) == 0) {
        printf("Listening\n");
    } else {
        printf("Error\n");
        return ECONNREFUSED;
    }

    int32_t num_of_threads = 0;

    struct sockaddr_storage server_storage;

    /* Incoming socket */
    int32_t new_sd;

    socklen_t addr_size;

    while (true) {
        /* Accept call. Create a new socket for the incoming connection */
        addr_size = sizeof(server_storage);
        printf("Wait client\n");
        new_sd = accept(server_sockfd, (struct sockaddr *)&server_storage,
                        &addr_size);
        printf("New client\n");

        opened_sockets[num_of_opened_sockets++] = new_sd;

        /*
         * For each client request creates a thread and assign the client
         * request to it to process so the main thread can entertain next
         * request
         */
        int32_t result = pthread_create(&threads[num_of_threads++], NULL,
                                        socket_thread, &new_sd);

        if (result != 0) {
            printf("Failed to create thread\n %i", result);
        }

        if (num_of_threads >= 60) {
            join_threads();
        }
    }

    return 0;
}
