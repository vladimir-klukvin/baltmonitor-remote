/**
 * @file session.h
 * @author V.K.
 * @brief
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
#ifndef SESSION_H_
#define SESSION_H_

#include <stdint.h>

#include "global.h"

/**
 * @brief Struct which contain state of pair communication session
 */
struct session_info {
    uint16_t id;
    bool_t is_host_connected;
    bool_t is_target_connected;
    int32_t host_sockfd;
    int32_t target_sockfd;
};

/**
 * @brief Get session by specified id
 * @param id Id number of session
 * @return struct session_info* Pointer to requested session
 */
struct session_info *session_get(uint16_t id);

/**
 * @brief Add new session with specified id
 * @param session Session to store
 * @param id Id of session
 */
extern void session_add(struct session_info session, uint16_t id);

/**
 * @brief Remove session by id. If session not found does nothing.
 * @param id Id of session
 */
extern void session_remove(uint16_t id);

/**
 * @brief Initialize sessions table to store max_sessions
 * @param max_sessions Max number of sessions which server supports
 */
extern void session_init_table(int16_t max_sessions);

/**
 * @brief Check session for existing in table
 * @param id Id of session
 * @return bool_t true if exist, false if not
 */
extern bool_t session_is_exist(uint16_t id);

#endif /* SESSION_H_ */
