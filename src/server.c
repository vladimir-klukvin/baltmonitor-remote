/**
 * @file server.c
 * @author V.K.
 * @brief
 * @date 2021-02-26
 *
 * @copyright Copyright. All rights reserved.
 *
 */
#include <arpa/inet.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global.h"
#include "log.h"
#include "session.h"

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

static int32_t server_sockfd;

static pthread_t threads[60];
static int32_t num_of_threads = 0;

static int32_t opened_sockets[60];
static int32_t num_of_opened_sockets = 0;

struct session_info test_info;

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

static void close_host_connection(struct session_info *session)
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

static void close_target_connection(struct session_info *session)
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

static void host_routine(struct session_info *session)
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

static void target_routine(struct session_info *session)
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

static uint16_t generate_session_id(void)
{
    uint16_t id;
    do {
        id = rand() % 10000;
    } while (session_is_exist(id));

    return id;
}

static struct session_info *new_session(int32_t hostfd)
{
    struct session_info *session = calloc(1, sizeof(struct session_info));
    session->host_sockfd = hostfd;
    session->is_host_connected = true;

    session->session_id = generate_session_id();

    return session;
}

static struct session_info *join_session(uint16_t id, int32_t targetfd)
{
    if (!session_is_exist(id)) {
        return NULL;
    }

    struct session_info *session = session_get(id);
    session->target_sockfd = targetfd;
    session->is_target_connected = true;

    return session;
}

static void clear_empty_session(struct session_info *session)
{
    if(!session->is_host_connected && !session->is_target_connected)
    {
        session_remove(session->session_id);
    }
}

static void *socket_thread(void *arg)
{
    int32_t sockfd = *((int32_t *)arg);

    char_t buffer[1000];
    recv(sockfd, buffer, sizeof(buffer), 0);
    struct request *req = (struct request *)buffer;
    struct session_info *session;
    switch (req->header.req_type) {
    case REQUEST_MAKE_SESSION:
        if (req->header.role != ROLE_HOST) {
            break;
        }
        session = new_session(sockfd);

        /* FIXME: TEST */
        send(sockfd, &session->session_id, sizeof(session->session_id), 0);

        host_routine(session);
        clear_empty_session(session);
        break;

    case REQUEST_JOIN_SESSION:
        if (req->header.role != ROLE_TARGET) {
            break;
        }

        session = join_session(req->header.session_id, sockfd);

        if(session == NULL){
            break;
        }

        target_routine(session);
        clear_empty_session(session);
        break;
    default:
        break;
    }

    log_debug("Exit socket_thread");
    close(sockfd);
    pthread_exit(NULL);
}

int32_t server_start(char_t *addr, uint16_t port, int32_t max_connections)
{
    /* Print server info message */
    log_info("Starting server: %s:%i", addr, port);
    log_info("Max connetions: %i", max_connections);

    /* Set seed for rand function */
    srand(time(0));

    /* Init sessions table */
    session_init_table(max_connections);

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
    server_addr.sin_port = htons(port);

    /* Set IP address */
    server_addr.sin_addr.s_addr = inet_addr(addr);

    /* Set all bits of the padding field to 0 */
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    /* Bind the address struct to the socket */
    if (bind(server_sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        log_error("Unable to bind");
        return ECONNREFUSED;
    }

    /* Listen on the socket, with max connection requests queued */
    if (listen(server_sockfd, max_connections) == 0) {
        log_debug("Listening");
    } else {
        log_error("Unable to listen");
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
        log_debug("Wait for a client");
        new_sd = accept(server_sockfd, (struct sockaddr *)&server_storage,
                        &addr_size);
        log_debug("Accept client");

        opened_sockets[num_of_opened_sockets++] = new_sd;

        /*
         * For each client request creates a thread and assign the client
         * request to it to process so the main thread can entertain next
         * request
         */
        int32_t result = pthread_create(&threads[num_of_threads++], NULL,
                                        socket_thread, &new_sd);

        if (result != 0) {
            log_error("Failed to create thread: %i", result);
        }

        if (num_of_threads >= max_connections) {
            join_threads();
        }
    }
}

void server_stop(void)
{
    close_sockets();
    close(server_sockfd);
    shutdown(server_sockfd, SHUT_RDWR);
    join_threads();
}
