/**
 * @file server.h
 * @author V.K.
 * @brief
 * @date 2021-02-26
 *
 * @copyright Copyright Balt-System Ltd. <info@bsystem.ru>
 * All rights reserved.
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
