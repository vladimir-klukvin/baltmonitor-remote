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
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

#include "global.h"
#include "log.h"
#include "session.h"

/**
 * @brief The types of response messages that the server send to clients
 */
enum response_type {
    RESPONSE_MAKE_SESSION_SUCCESS,
    RESPONSE_MAKE_SESSION_FAIL,
    RESPONSE_JOIN_SESSION_SUCCESS,
    RESPONSE_JOIN_SESSION_FAIL,
    RESPONSE_SESSION_CLOSED_BY_HOST,
    RESPONSE_SESSION_CLOSED_BY_TARGET,
    RESPONSE_RAISE_EVENT,
    RESPONSE_DATA,
    RESPONSE_BAD_REQUEST
};

/**
 * @brief The types of requests that clients send to the server
 */
enum request_type {
    REQUEST_MAKE_SESSION,
    REQUEST_JOIN_SESSION,
    REQUEST_CLOSE_SESSION,
    REQUEST_RAISE_EVENT,
    REQUEST_DATA
};

/**
 * @brief Roles of clients in the session. The creator of the session is the
 * host, the one who connected to the session is the target.
 */
enum role { ROLE_HOST, ROLE_TARGET };

/**
 * @brief Structure of the response.
 * The header contains service information and the body contains response data.
 */
struct response {
    struct response_header {
        enum response_type type : 8;
        uint16_t session_id;
        /*
         * Since the size of the 'body' field can be different this field is
         * used to indicate its size
         */
        size_t body_size;
    } header;
    /*
     * This field is never used by the server.
     * contains any information that will be used by clients.
     * Used with response types RESPONSE_DATA and RESPONSE_RAISE_EVENT.
     */
    uint8_t body[];
};

/**
 * @brief Structure of the request.
 * The header contains service information and the body contains request data.
 */
struct request {
    struct request_header {
        enum request_type type : 8;
        enum role role : 8;
        uint16_t session_id;
        /*
         * Since the size of the 'body' field can be different this field is
         * used to indicate its size
         */
        size_t body_size;
    } header;
    /*
     * This field is never used by the server.
     * contains any information that will be used by clients.
     * Used with request types REQUEST_DATA and REQUEST_RAISE_EVENT.
     */
    uint8_t body[];
};

/* The size of the socket buffer used to store the request from the host */
static const size_t host_socket_buffer_size = 150000;

/* The size of the socket buffer used to store the request from the target */
static const size_t target_socket_buffer_size = 1000;

/* Server's socket file descriptor */
static int32_t server_sockfd;

/* Active working threads, one per client */
/* TODO: Use linked list */
static pthread_t threads[60];

/* Number of active working threads */
static int32_t num_of_threads = 0;

/* Active opened sockets, one per client */
/* TODO: Use linked list or get sockets from sessions */
static int32_t opened_sockets[60];

/* Number of active opened sockets */
static int32_t num_of_opened_sockets = 0;

/**
 * @brief Join all active socket threads
 */
void join_threads(void)
{
    for (int32_t i = 0; i < num_of_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    num_of_threads = 0;
}

/**
 * @brief Shutdown and close all opened sockets
 */
void close_sockets(void)
{
    for (int32_t i = 0; i < num_of_opened_sockets; i++) {
        shutdown(opened_sockets[i], SHUT_RDWR);
        close(opened_sockets[i]);
    }

    num_of_opened_sockets = 0;
}

/**
 * @brief Initiate a session to be closed by the host. If the target is still
 * connected, then send a notification to it.
 * @param session The specified session to be closed
 */
static void host_leave_session(struct session_info *session)
{
    session->is_host_connected = false;

    if (session->is_target_connected) {
        struct response resp;
        resp.header.body_size = 0;
        resp.header.type = RESPONSE_SESSION_CLOSED_BY_HOST;
        resp.header.session_id = session->id;
        send(session->target_sockfd, &resp, sizeof(resp), 0);
    }
}

/**
 * @brief Initiate a session to be closed by the target. If the host is still
 * connected, then send a notification to it.
 * @param session The specified session to be closed
 */
static void target_leave_session(struct session_info *session)
{
    session->is_target_connected = false;

    if (session->is_host_connected) {
        struct response resp;
        resp.header.body_size = 0;
        resp.header.type = RESPONSE_SESSION_CLOSED_BY_TARGET;
        resp.header.session_id = session->id;

        send(session->host_sockfd, &resp, sizeof(resp), 0);
    }
}

/**
 * @brief After receiving a bad request, send a response with information about
 * it.
 * @param sockfd Socket file descriptor of the bad request sender
 * @param session_id The session within which the bad request was received
 */
static void send_bad_request(int32_t sockfd, uint16_t session_id)
{
    struct response resp;
    resp.header.body_size = 0;
    resp.header.type = RESPONSE_BAD_REQUEST;
    resp.header.session_id = session_id;

    send(sockfd, &resp, sizeof(resp), 0);
}

static bool_t is_socket_error(enum role role, ssize_t req_size)
{
    /*
     * If the size is less than or equal to 0, then this is a socket error,
     * usually this happens when the socket is closed. The size of the buffer
     * was chosen in such a way that the largest theoretically possible request
     * would fit into it, therefore, too large requests are also incorrect.
     * Well, if the request is less than the length of the header, then there is
     * some problem with the socket.
     */

    size_t max_size = role == ROLE_HOST ? host_socket_buffer_size :
                                          target_socket_buffer_size;

    return (req_size <= 0) || (req_size == (ssize_t)max_size) ||
           (req_size < (ssize_t)sizeof(struct request));
}

static bool_t is_bad_request(enum role role, uint16_t session_id,
                             const struct request *req, ssize_t req_size)
{
    if (req->header.role != role) {
        return true;
    }

    if (req->header.session_id != session_id) {
        return true;
    }

    size_t expected_size =
        req->header.body_size + sizeof(struct request_header);

    if (req_size != (ssize_t)expected_size) {
        return true;
    }

    return false;
}

static void host_routine(struct session_info *session)
{
    session->is_host_connected = true;
    struct request *req = malloc(host_socket_buffer_size);

    while (session->is_host_connected) {
        ssize_t req_size =
            recv(session->host_sockfd, req, host_socket_buffer_size, 0);

        if (is_socket_error(ROLE_HOST, req_size)) {
            host_leave_session(session);
            continue;
        }

        if (is_bad_request(ROLE_HOST, session->id, req, req_size)) {
            send_bad_request(session->host_sockfd, session->id);
            continue;
        }

        switch (req->header.type) {
        case REQUEST_CLOSE_SESSION:
            host_leave_session(session);
            break;
        case REQUEST_DATA:
            /* TODO: Make some response and send */
            break;
        case REQUEST_RAISE_EVENT:
            /* TODO: Make some response and send */
            break;

        case REQUEST_MAKE_SESSION:
        case REQUEST_JOIN_SESSION:
        default:
            send_bad_request(session->host_sockfd, session->id);
            break;
        }
    }
    free(req);
}

static void target_routine(struct session_info *session)
{
    session->is_target_connected = true;
    struct request *req = malloc(target_socket_buffer_size);

    while (session->is_target_connected) {
        ssize_t req_size =
            recv(session->target_sockfd, req, target_socket_buffer_size, 0);

        if (is_socket_error(ROLE_TARGET, req_size)) {
            target_leave_session(session);
            continue;
        }

        if (is_bad_request(ROLE_TARGET, session->id, req, req_size)) {
            send_bad_request(session->target_sockfd, session->id);
            continue;
        }

        switch (req->header.type) {
        case REQUEST_CLOSE_SESSION:
            target_leave_session(session);
            break;
        case REQUEST_DATA:
            /* TODO: Make some response and send */
            break;
        case REQUEST_RAISE_EVENT:
            /* TODO: Make some response and send */
            break;

        case REQUEST_MAKE_SESSION:
        case REQUEST_JOIN_SESSION:
        default:
            send_bad_request(session->target_sockfd, session->id);
            break;
        }
    }
    free(req);
}

static uint16_t generate_session_id(void)
{
    uint16_t id;
    do {
        id = (uint16_t)(rand() % 10000);
    } while (session_is_exist(id));

    return id;
}

static struct session_info *new_session(int32_t host_sockfd)
{
    struct session_info *session = calloc(1, sizeof(struct session_info));
    session->host_sockfd = host_sockfd;
    session->is_host_connected = true;

    session->id = generate_session_id();

    log_info("New session with id %i created", session->id);

    return session;
}

static struct session_info *join_session(uint16_t id, int32_t target_sockfd)
{
    if (!session_is_exist(id)) {
        return NULL;
    }

    struct session_info *session = session_get(id);
    session->target_sockfd = target_sockfd;
    session->is_target_connected = true;

    log_info("Joining to session with id %i success", session->id);

    return session;
}

static void clear_empty_session(struct session_info *session)
{
    if (!session->is_host_connected && !session->is_target_connected) {
        session_remove(session->id);
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
            send_session_response(sockfd, RESPONSE_MAKE_SESSION_FAIL, 0);
            break;
        }

        struct session_info *session = new_session(sockfd);

        send_session_response(sockfd, RESPONSE_MAKE_SESSION_SUCCESS,
                              session->id);

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
                              session->id);

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

noreturn void server_start(const char_t *addr, uint16_t port,
                           int32_t max_clients)
{
    /* Print server info message */
    log_info("Starting server: %s:%i", addr, port);
    log_info("Max connections: %i", max_clients);

    /* Set seed for rand function */
    srand((uint16_t)time(NULL));

    /* Init sessions table */
    session_init_table((uint16_t)max_clients);

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
        exit(EXIT_FAILURE);
    }

    /* Listen on the socket, with max connection requests queued */
    if (listen(server_sockfd, max_clients) == 0) {
        log_debug("Listening");
    } else {
        log_error("Unable to listen");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

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
