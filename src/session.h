/**
 * @file session.h
 * @author V.K.
 * @brief
 * @date 2021-02-26
 *
 * @copyright Copyright. All rights reserved.
 *
 */
#ifndef SESSION_H_
#define SESSION_H_

#include <stdint.h>
#include <stdbool.h>

struct session_info {
    uint16_t session_id;
    bool is_host_connected;
    bool is_target_connected;
    int32_t host_sockfd;
    int32_t target_sockfd;
};

struct session_info *session_get(uint16_t id);

void session_add(struct session_info session, uint16_t id);

void session_remove(uint16_t id);

void session_init_table(int16_t max_sessions);

#endif /* SESSION_H_ */
