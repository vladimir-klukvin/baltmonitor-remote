/**
 * @file log.h
 * @brief This file contains function declarations for logging (including
 * logging configuration)
 *
 * @author Vladimir Klukvin <vladimir.klukvin@yandex.com>
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
 */
#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>

#include "global.h"

/**
 * @brief Available log levels
 */
enum log_level {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};

/**
 * @brief Writes the diagnostic message at the @b Error level using the
 * specified parameters and formatting them with the supplied format string.
 * @param format Format string
 * @param ... Arguments to format
 */
extern void log_error(const char_t *format, ...);

/**
 * @brief Writes the diagnostic message at the @b Warning level using the
 * specified parameters and formatting them with the supplied format string.
 * @param format Format string
 * @param ... Arguments to format
 */
extern void log_warning(const char_t *format, ...);

/**
 * @brief Writes the diagnostic message at the @b Information level using the
 * specified parameters and formatting them with the supplied format string.
 * @param format Format string
 * @param ... Arguments to format
 */
extern void log_info(const char_t *format, ...);

/**
 * @brief Writes the diagnostic message at the @b Debug level using the
 * specified parameters and formatting them with the supplied format string.
 * @param format Format string
 * @param ... Arguments to format
 */
extern void log_debug(const char_t *format, ...);

/**
 * @brief Set the minimum logging level, all calls of logging functions of
 * a lower level will have no effect
 * @param level Minimum logging level
 */
extern void log_set_min_level(enum log_level level);

/**
 * @brief Reset internal state and set syslog as default target
 */
extern void log_reset_state(void);

/**
 * @brief Set log target to specified file
 * @param filename The name of the file where the logs will be sent.
 * If the file already exists, it will be appended.
 * @return int32_t 0 for success or -1 for errors
 */
extern int32_t log_set_log_file(const char_t *filename);

/**
 * @brief Set standard output as log target
 */
extern void log_set_out_stdout(void);

#endif /* LOG_H_ */
