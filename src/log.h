/**
 * @file log.h
 * @author V.K.
 * @brief
 * @date 2021-03-01
 *
 * @copyright Copyright Balt-System Ltd. <info@bsystem.ru>
 * All rights reserved.
 *
 */
#ifndef LOG_H_
#define LOG_H_

/*
 * Logging methods by levels
 */
extern void log_error(char_t *format, ...);
extern void log_warning(char_t *format, ...);
extern void log_info(char_t *format, ...);
extern void log_debug(char_t *format, ...);

/*
 * Log level configurator
 * Default is LOG_MAX_LEVEL_ERROR_WARNING_INFO
 */
#define LOG_MAX_LEVEL_ERROR                    0
#define LOG_MAX_LEVEL_ERROR_WARNING_INFO       1
#define LOG_MAX_LEVEL_ERROR_WARNING_INFO_DEBUG 2

extern void log_set_log_level(const int32_t level);

/*
 * Set target type
 * Default is syslog
 */
extern void log_reset_state(void);
extern int32_t log_set_log_file(const char_t *filename);
extern void log_set_out_stdout(void);

#endif /* LOG_H_ */
