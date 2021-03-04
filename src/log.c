/**
 * @file log.c
 * @author V.K.
 * @brief This file contains function definitions and auxiliary data structures
 * for logging.
 * @date 2021-03-01
 *
 * @copyright Copyright Balt-System Ltd. <info@bsystem.ru>
 * All rights reserved.
 *
 */
#include "log.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "global.h"

static enum log_level min_log_level;

static bool_t use_stdout;

static FILE *out_file;

static void (*logger_func)(enum log_level level, const char_t *message);

/**
 * @brief Prefixes for the different logging levels
 */
static const char_t *log_level_prefixes[] = {[LOG_LEVEL_ERROR] = "ERROR",
                                             [LOG_LEVEL_WARNING] = "WARNING",
                                             [LOG_LEVEL_INFO] = "INFO",
                                             [LOG_LEVEL_DEBUG] = "DEBUG"};

static void print_to_syslog(enum log_level level, const char_t *message);
static void print_to_file(enum log_level level, const char_t *message);

/**
 * @brief Close remaining file descriptor and reset global params
 */
static void cleanup_internal(void)
{
    if (out_file) {
        if (!use_stdout) {
            fclose(out_file);
        }

        use_stdout = false;
        out_file = NULL;
    }
}

/**
 * @brief Reset internal state and set syslog as default target
 */
void log_reset_state(void)
{
    min_log_level = LOG_LEVEL_INFO;
    cleanup_internal();
    logger_func = print_to_syslog;
}

/**
 * @brief Print to syslog
 */
static void print_to_syslog(enum log_level level, const char_t *message)
{
    syslog(LOG_INFO, "[%s] %s\n", log_level_prefixes[level], message);
}

/**
 * @brief Print to file which can be a regular text file or STDOUT "file"
 */
static void print_to_file(enum log_level level, const char_t *message)
{
    struct tm *current_tm;
    time_t time_now;

    time(&time_now);
    current_tm = localtime(&time_now);

    int32_t res = fprintf(
        out_file, "%s: %04i-%02i-%02i %02i:%02i:%02i [%s] %s\n", PROGRAM_NAME,
        current_tm->tm_year + 1900, current_tm->tm_mon + 1, current_tm->tm_mday,
        current_tm->tm_hour, current_tm->tm_min, current_tm->tm_sec,
        log_level_prefixes[level], message);

    if (res == -1) {
        print_to_syslog(LOG_LEVEL_ERROR, "Unable to write to log file");
        return;
    }

    fflush(out_file);
}

void log_set_min_level(enum log_level level)
{
    min_log_level = level;
}

int32_t log_set_log_file(const char_t *filename)
{
    cleanup_internal();

    out_file = fopen(filename, "a");

    if (out_file == NULL) {
        log_error("Failed to open file %s error %s", filename, strerror(errno));
        return -1;
    }

    logger_func = print_to_file;

    return 0;
}

void log_set_out_stdout(void)
{
    cleanup_internal();

    use_stdout = true;
    logger_func = print_to_file;
    out_file = stdout;
}

static void log_generic(enum log_level level, const char_t *format,
                        va_list args)
{
    char_t buffer[256];
    vsprintf(buffer, format, args);
    logger_func(level, buffer);
}

void log_error(const char_t *format, ...)
{
    va_list args;
    va_start(args, format);
    log_generic(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}

void log_warning(const char_t *format, ...)
{
    if (min_log_level < LOG_LEVEL_WARNING) {
        return;
    }

    va_list args;
    va_start(args, format);
    log_generic(LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

void log_info(const char_t *format, ...)
{
    if (min_log_level < LOG_LEVEL_INFO) {
        return;
    }

    va_list args;
    va_start(args, format);
    log_generic(LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void log_debug(const char_t *format, ...)
{
    if (min_log_level < LOG_LEVEL_DEBUG) {
        return;
    }

    va_list args;
    va_start(args, format);
    log_generic(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}
