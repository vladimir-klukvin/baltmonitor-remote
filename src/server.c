/**
 * @file server.c
 * @author V.K.
 * @brief This file contains definitions of functions containing the logic of
 * the remote server, including the request exchange protocol and server
 * management.
 * @date 2021-02-26
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
    RESPONSE_RAISE_EVENT = 'E',
    RESPONSE_DATA = 'D',
};

enum request_type {
    REQUEST_MAKE_SESSION = 'M',
    REQUEST_JOIN_SESSION = 'J',
    REQUEST_CLOSE_SESSION = 'C',
    REQUEST_RAISE_EVENT = 'E',
    REQUEST_DATA = 'D',
};

enum role { ROLE_HOST = 'H', ROLE_TARGET = 'T' };

struct response {
    struct response_header {
        enum response_type type;
        uint16_t session_id;
        size_t body_size;
    } header;
    uint8_t body[];
};

struct request {
    struct request_header {
        enum request_type type : 8;
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

static void host_leave_session(struct session_info *session)
{
    session->is_host_connected = false;

    if (session->is_target_connected) {
        struct response resp;
        resp.header.body_size = 0;
        resp.header.type = RESPONSE_SESSION_CLOSED_BY_HOST;
        resp.header.session_id = session->session_id;
        send(session->target_sockfd, &resp, sizeof(resp), 0);
    }
}

static void target_leave_session(struct session_info *session)
{
    session->is_target_connected = false;

    if (session->is_host_connected) {
        struct response resp;
        resp.header.body_size = 0;
        resp.header.type = RESPONSE_SESSION_CLOSED_BY_TARGET;
        resp.header.session_id = session->session_id;
        send(session->host_sockfd, &resp, sizeof(resp), 0);
    }
}

static void host_routine(struct session_info *session)
{
    session->is_host_connected = true;
    struct request *req = malloc(SOCKET_BUFFER_SIZE);

    while (session->is_host_connected) {
        ssize_t req_size =
            recv(session->host_sockfd, req, SOCKET_BUFFER_SIZE, 0);

        if (req_size <= 0|| req_size == SOCKET_BUFFER_SIZE) {
            host_leave_session(session);
            continue;
        }

        if (req->header.role != ROLE_HOST) {
            /* TODO: Send bad request */
        }

        size_t expected_size =
            req->header.body_size + sizeof(struct request_header);

        if (req->header.session_id != session->session_id) {
            /* TODO: Send bad request */
        }

        if (req_size != expected_size) {
            /* TODO: Send bad request */
        }

        switch (req->header.type) {
        case REQUEST_CLOSE_SESSION:
            host_leave_session(session);
            continue;
        case REQUEST_DATA:
            /* TODO: Make some response and send */
            continue;
        case REQUEST_RAISE_EVENT:
            /* TODO: Make some response and send */
            continue;

        case REQUEST_MAKE_SESSION:
        case REQUEST_JOIN_SESSION:
        default:
            /* FIXME: Send something about bad request? */
            host_leave_session(session);
            continue;
        }
    }
    free(req);
}

static void target_routine(struct session_info *session)
{
    session->is_target_connected = true;
    struct request *req = malloc(SOCKET_BUFFER_SIZE);

    while (session->is_target_connected) {
        size_t req_size =
            recv(session->target_sockfd, req, SOCKET_BUFFER_SIZE, 0);

        if (req_size <= 0 || req_size == SOCKET_BUFFER_SIZE) {
            target_leave_session(session);
            continue;
        }

        if (req->header.role != ROLE_TARGET) {
            /* TODO: Send bad request */
        }

        size_t expected_size =
            req->header.body_size + sizeof(struct request_header);

        if (req->header.session_id != session->session_id) {
            /* TODO: Send bad request */
        }

        if (req_size != expected_size) {
            /* TODO: Send bad request */
        }

        switch (req->header.type) {
        case REQUEST_CLOSE_SESSION:
            host_leave_session(session);
            continue;
        case REQUEST_DATA:
            /* TODO: Make some response and send */
            continue;
        case REQUEST_RAISE_EVENT:
            /* TODO: Make some response and send */
            continue;

        case REQUEST_MAKE_SESSION:
        case REQUEST_JOIN_SESSION:
        default:
            /* FIXME: Send something about bad request? */
            host_leave_session(session);
            continue;
        }
    }
    free(req);
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

    log_info("New session with id %i created", session->session_id);

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

    log_info("Joining to session with id %i success", session->session_id);

    return session;
}

static void clear_empty_session(struct session_info *session)
{
    if (!session->is_host_connected && !session->is_target_connected) {
        session_remove(session->session_id);
    }
}

static void send_session_response(int32_t sockfd, enum response_type type,
                                  uint16_t session_id)
{
    struct response resp = {.header.type = type,
                            .header.session_id = session_id};

    send(sockfd, &resp, sizeof(resp), 0);
}

static void handle_session_request(const struct request *req, int32_t sockfd)
{
    switch (req->header.type) {
    case REQUEST_MAKE_SESSION: {
        if (req->header.role != ROLE_HOST) {
            send_session_response(sockfd, RESPONCE_MAKE_SESSION_FAIL, 0);
            break;
        }

        struct session_info *session = new_session(sockfd);

        send_session_response(sockfd, RESPONCE_MAKE_SESSION_SUCCESS,
                              session->session_id);

        host_routine(session);
        clear_empty_session(session);
        break;
    }

    case REQUEST_JOIN_SESSION: {
        if (req->header.role != ROLE_TARGET) {
            send_session_response(sockfd, RESPONSE_JOIN_SESSION_FAIL,
                                  req->header.session_id);
            break;
        }

        struct session_info *session =
            join_session(req->header.session_id, sockfd);

        if (session == NULL) {
            send_session_response(sockfd, RESPONSE_JOIN_SESSION_FAIL,
                                  req->header.session_id);
            break;
        }

        send_session_response(sockfd, RESPONSE_JOIN_SESSION_SUCCESS,
                              session->session_id);

        target_routine(session);
        clear_empty_session(session);
        break;
    }
    default:
        break;
    }
}

static void *socket_thread(void *arg)
{
    int32_t sockfd = *((int32_t *)arg);

    char_t buffer[1000];
    ssize_t recv_size = recv(sockfd, buffer, sizeof(buffer), 0);

    if (recv_size == sizeof(struct request_header)) {
        struct request *req = (struct request *)buffer;

        handle_session_request(req, sockfd);
    }

    log_debug("Exit socket_thread");
    close(sockfd);
    pthread_exit(NULL);
}

int32_t server_start(const char_t *addr, uint16_t port, int32_t max_clients)
{
    /* Print server info message */
    log_info("Starting server: %s:%i", addr, port);
    log_info("Max connetions: %i", max_clients);

    /* Set seed for rand function */
    srand(time(0));

    /* Init sessions table */
    session_init_table(max_clients);

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
        close(server_sockfd);
        log_error("Unable to bind");
        return ECONNREFUSED;
    }

    /* Listen on the socket, with max connection requests queued */
    if (listen(server_sockfd, max_clients) == 0) {
        log_debug("Listening");
    } else {
        log_error("Unable to listen");
        close(server_sockfd);
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

        if (num_of_threads >= max_clients) {
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
