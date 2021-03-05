/**
 * @file server.h
 * @author V.K.
 * @brief This file contains function declarations for managing the remote
 * server.
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
#ifndef SERVER_H_
#define SERVER_H_

#include <stdint.h>

#include "global.h"

/**
 * @brief Start remote server
 * @param addr Server IP address
 * @param port Server will listen specified port
 * @param max_clients Can serve simultaneously clients
 * @return int32_t Status error for exit() (noreturn in nornal way).
 */
int32_t server_start(const char_t *addr, uint16_t port, int32_t max_clients);

/**
 * @brief Stop remote server.
 */
void server_stop(void);

#endif /* SERVER_H_ */
