/**
 * @file log.h
 * @author V.K.
 * @brief This file contains function declarations for logging (including
 * logging configuringuration)
 * @date 2021-03-01
 *
 * @copyright Copyright Balt-System Ltd. <info@bsystem.ru>
 * All rights reserved.
 *
 */
#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>

#include "global.h"

/*
 * Logging methods by levels
 */
extern void log_error(const char_t *format, ...);
extern void log_warning(const char_t *format, ...);
extern void log_info(const char_t *format, ...);
extern void log_debug(const char_t *format, ...);

enum log_level {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};

extern void log_set_min_level(enum log_level level);
extern void log_reset_state(void);
extern int32_t log_set_log_file(const char_t *filename);
extern void log_set_out_stdout(void);

#endif /* LOG_H_ */
