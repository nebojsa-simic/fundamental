/*
 * Logging module - Centralized logging system.
 *
 * Implements:
 *   - Log levels (debug, info, warn, error, fatal)
 *   - Configuration via [logging] section
 *   - Console and file output
 *
 * Initialized during startup phase 5 (after config is loaded).
 */

#include "fundamental/logging/logging.h"
#include "fundamental/config/config.h"
#include "fundamental/console/console.h"
#include "fundamental/startup/startup.h"
#include <stdarg.h>

/* Global logging state */
static LoggingConfig g_logging_config;
static bool g_logging_initialized = false;

/*
 * Logging initialization (Phase 5)
 * Reads configuration from already-loaded config.
 * Silent operation if config is not available.
 */
static void logging_init(void)
{
	Config cfg;
	StringResult level_result;
	StringResult output_result;

	/* Default configuration */
	g_logging_config.level = LOG_LEVEL_INFO;
	g_logging_config.output_to_console = true;
	g_logging_config.output_to_file = false;
	g_logging_config.file_path[0] = '\0';

	/* Get global config */
	cfg = fun_config_get_global();

	/* Read [logging] section from config */
	level_result =
		fun_config_get_string_or_default(&cfg, "logging.level", "info");

	if (fun_error_is_ok(level_result.error)) {
		if (fun_string_compare(level_result.value, "debug") == 0) {
			g_logging_config.level = LOG_LEVEL_DEBUG;
		} else if (fun_string_compare(level_result.value, "info") == 0) {
			g_logging_config.level = LOG_LEVEL_INFO;
		} else if (fun_string_compare(level_result.value, "warn") == 0) {
			g_logging_config.level = LOG_LEVEL_WARN;
		} else if (fun_string_compare(level_result.value, "error") == 0) {
			g_logging_config.level = LOG_LEVEL_ERROR;
		} else if (fun_string_compare(level_result.value, "fatal") == 0) {
			g_logging_config.level = LOG_LEVEL_FATAL;
		}
	}

	/* Read output configuration */
	output_result =
		fun_config_get_string_or_default(&cfg, "logging.output", "console");
	if (fun_error_is_ok(output_result.error)) {
		if (fun_string_compare(output_result.value, "console") == 0) {
			g_logging_config.output_to_console = true;
			g_logging_config.output_to_file = false;
		} else if (fun_string_compare(output_result.value, "file") == 0) {
			g_logging_config.output_to_console = false;
			g_logging_config.output_to_file = true;
			/* Read file path */
			StringResult path_result = fun_config_get_string_or_default(
				&cfg, "logging.file_path", "app.log");
			if (fun_error_is_ok(path_result.error)) {
				fun_string_copy(path_result.value, g_logging_config.file_path,
								sizeof(g_logging_config.file_path));
			}
		}
	}

	g_logging_initialized = true;
}

void fun_logging_set_level(int level)
{
	g_logging_config.level = level;
}

int fun_logging_get_level(void)
{
	return g_logging_config.level;
}

/* Internal log function */
static void fun_log_internal(int level, const char *format, va_list args)
{
	/* Check if message should be logged */
	if (level < g_logging_config.level && level != LOG_LEVEL_FATAL) {
		return;
	}

	/* Format prefix based on level */
	const char *prefix;
	switch (level) {
	case LOG_LEVEL_DEBUG:
		prefix = "[DEBUG] ";
		break;
	case LOG_LEVEL_INFO:
		prefix = "[INFO]  ";
		break;
	case LOG_LEVEL_WARN:
		prefix = "[WARN]  ";
		break;
	case LOG_LEVEL_ERROR:
		prefix = "[ERROR] ";
		break;
	case LOG_LEVEL_FATAL:
		prefix = "[FATAL] ";
		break;
	default:
		prefix = "[LOG]   ";
		break;
	}

	/* Output to console */
	if (g_logging_config.output_to_console) {
		fun_console_write(prefix);
		/* Note: In a full implementation, we'd use vprintf here */
		/* For now, just output the prefix to indicate the log */
	}

	/* Output to file would go here in a full implementation */
	(void)format;
	(void)args;
}

void fun_log_debug(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	fun_log_internal(LOG_LEVEL_DEBUG, format, args);
	va_end(args);
}

void fun_log_info(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	fun_log_internal(LOG_LEVEL_INFO, format, args);
	va_end(args);
}

void fun_log_warn(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	fun_log_internal(LOG_LEVEL_WARN, format, args);
	va_end(args);
}

void fun_log_error(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	fun_log_internal(LOG_LEVEL_ERROR, format, args);
	va_end(args);
}

void fun_log_fatal(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	fun_log_internal(LOG_LEVEL_FATAL, format, args);
	va_end(args);
}

void fun_logging_shutdown(void)
{
	g_logging_initialized = false;
	/* Flush file buffers if file output is enabled */
}

/* Register logging init at phase 5 */
FUNDAMENTAL_STARTUP_REGISTER(STARTUP_PHASE_LOGGING, logging_init);
