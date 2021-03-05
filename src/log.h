/**
 * @file log.h
 * @author V.K.
 * @brief This file contains function declarations for logging (including
 * logging configuringuration)
 * @date 2021-03-01
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
