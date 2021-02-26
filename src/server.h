/**
 * @file server.h
 * @author Zh.T. V.K.
 * @brief
 * @date 2021-02-26
 *
 * @copyright Copyright. All rights reserved.
 *
 */
#ifndef SERVER_H_
#define SERVER_H_

#include <stdint.h>

#include "global.h"

int32_t server_start(char_t *addr, uint16_t port, int32_t max_connections);

void server_stop(void);

#endif /* SERVER_H_ */
