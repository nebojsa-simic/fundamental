#ifndef LIBRARY_LOGGING_H
#define LIBRARY_LOGGING_H

#include "../string/string.h"
#include "../config/config.h"

/*
 * Logging Module - Centralized Logging System
 *
 * Provides logging functionality with configurable log levels.
 * Initialized during startup phase 5 (after config is loaded).
 *
 * Log Levels:
 *   LOG_LEVEL_DEBUG   - Debug messages (lowest priority)
 *   LOG_LEVEL_INFO    - Informational messages
 *   LOG_LEVEL_WARN    - Warning messages
 *   LOG_LEVEL_ERROR   - Error messages
 *   LOG_LEVEL_FATAL   - Fatal error messages (highest priority)
 *
 * Usage:
 *   // Log messages at different levels
 *   fun_log_debug("Debug message: %d", value);
 *   fun_log_info("Info message");
 *   fun_log_warn("Warning: %s", msg);
 *   fun_log_error("Error: %s", error_msg);
 *   fun_log_fatal("Fatal error");
 *
 * Configuration:
 *   [logging]
 *   level = info          ; Minimum log level (debug/info/warn/error/fatal)
 *   output = console      ; Output destination (console/file)
 *   file_path = app.log   ; Path to log file (if output=file)
 */

/* Log level constants */
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 4

/* Logging configuration */
typedef struct {
	int level;
	bool output_to_console;
	bool output_to_file;
	char file_path[256];
} LoggingConfig;

/*
 * Initialize logging subsystem (Phase 5).
 *
 * Reads configuration from already-loaded config:
 *   [logging] section with level, output settings
 *
 * @return 0 on success, non-zero on failure
 */
int fun_logging_init(void);

/*
 * Set the minimum log level.
 *
 * Messages below this level will not be logged.
 *
 * @param level Minimum log level (LOG_LEVEL_DEBUG through LOG_LEVEL_FATAL)
 */
void fun_logging_set_level(int level);

/*
 * Get the current log level.
 *
 * @return Current minimum log level
 */
int fun_logging_get_level(void);

/*
 * Log a debug message.
 *
 * @param format Format string (printf-style)
 * @param ... Format arguments
 */
void fun_log_debug(const char *format, ...);

/*
 * Log an informational message.
 *
 * @param format Format string (printf-style)
 * @param ... Format arguments
 */
void fun_log_info(const char *format, ...);

/*
 * Log a warning message.
 *
 * @param format Format string (printf-style)
 * @param ... Format arguments
 */
void fun_log_warn(const char *format, ...);

/*
 * Log an error message.
 *
 * @param format Format string (printf-style)
 * @param ... Format arguments
 */
void fun_log_error(const char *format, ...);

/*
 * Log a fatal error message.
 *
 * This level is always logged regardless of configured level.
 *
 * @param format Format string (printf-style)
 * @param ... Format arguments
 */
void fun_log_fatal(const char *format, ...);

/*
 * Shutdown logging subsystem.
 *
 * Flushes any buffered output and closes file handles.
 */
void fun_logging_shutdown(void);

#endif /* LIBRARY_LOGGING_H */
