// Log Analyzer Demo - Fundamental Library
// Proper streaming implementation with chunked reading

#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/async/async.h"
#include "fundamental/hashmap/hashmap.h"
#include "fundamental/stream/stream.h"

DEFINE_HASHMAP_STRING_KEY(int64_t)

typedef struct {
	int64_t info;
	int64_t warn;
	int64_t error;
	int64_t debug;
	int64_t trace;
	int64_t unknown;
	int64_t total_lines;
	int64_t malformed_lines;
} LogStats;

static void parse_log_line(String line, LogStats *stats)
{
	stats->total_lines++;

	if (line[0] != '[') {
		stats->malformed_lines++;
		return;
	}

	StringPosition bracket_close = fun_string_index_of(line, "]", 0);
	if (bracket_close < 0) {
		stats->malformed_lines++;
		return;
	}

	StringPosition level_start = bracket_close + 2;
	StringLength line_len = fun_string_length(line);
	if (level_start >= (int64_t)line_len || line[level_start] != '[') {
		stats->malformed_lines++;
		return;
	}

	level_start++;
	StringPosition level_end = fun_string_index_of(line, "]", level_start);
	if (level_end < 0) {
		stats->malformed_lines++;
		return;
	}

	char level_buf[16];
	StringLength level_len = (StringLength)(level_end - level_start);
	if (level_len >= sizeof(level_buf)) {
		stats->malformed_lines++;
		return;
	}

	fun_string_substring(line, level_start, level_len, level_buf,
						 sizeof(level_buf));

	if (fun_string_compare(level_buf, "INFO") == 0) {
		stats->info++;
	} else if (fun_string_compare(level_buf, "WARN") == 0) {
		stats->warn++;
	} else if (fun_string_compare(level_buf, "ERROR") == 0) {
		stats->error++;
	} else if (fun_string_compare(level_buf, "DEBUG") == 0) {
		stats->debug++;
	} else if (fun_string_compare(level_buf, "TRACE") == 0) {
		stats->trace++;
	} else {
		stats->unknown++;
	}
}

static void parse_chunk(char *buffer, uint64_t bytes_read, LogStats *stats,
						char *carry_over, uint64_t *carry_len,
						uint64_t carry_max)
{
	uint64_t line_start = 0;

	for (uint64_t i = 0; i < bytes_read; i++) {
		if (buffer[i] == '\n' || buffer[i] == '\r') {
			if (i > line_start || *carry_len > 0) {
				char line_buf[8192];
				uint64_t line_len = 0;

				if (*carry_len > 0) {
					fun_memory_copy(carry_over, line_buf, *carry_len);
					line_len = *carry_len;
					*carry_len = 0;
				}

				uint64_t chunk_len = i - line_start;
				if (line_len + chunk_len < carry_max) {
					fun_memory_copy(&buffer[line_start], &line_buf[line_len],
									chunk_len);
					line_len += chunk_len;
					line_buf[line_len] = '\0';

					if (line_len > 0) {
						parse_log_line(line_buf, stats);
					}
				}
			}
			line_start = i + 1;
			if (buffer[i] == '\r' && i + 1 < bytes_read &&
				buffer[i + 1] == '\n') {
				line_start++;
				i++;
			}
		}
	}

	if (line_start < bytes_read) {
		uint64_t remaining = bytes_read - line_start;
		if (*carry_len + remaining < carry_max) {
			fun_memory_copy(&buffer[line_start], &carry_over[*carry_len],
							remaining);
			*carry_len += remaining;
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fun_console_write_line("=== Log Analyzer Demo ===");
		fun_console_write_line("Usage: demo.exe <logfile>");
		fun_console_write_line("");
		fun_console_write_line(
			"Analyzes log files and counts entries by level.");
		fun_console_write_line("Supports format: [TIMESTAMP] [LEVEL] message");
		return 1;
	}

	String file_path = argv[1];

	const uint64_t CHUNK_SIZE = 8192;
	MemoryResult buffer_result = fun_memory_allocate(CHUNK_SIZE);
	if (fun_error_is_error(buffer_result.error)) {
		fun_console_write_line("Failed to allocate buffer");
		return 1;
	}
	Memory buffer = buffer_result.value;

	AsyncResult open_result = fun_stream_create_file_read(
		file_path, buffer, CHUNK_SIZE, FILE_MODE_AUTO);
	fun_async_await(&open_result, -1);

	if (fun_error_is_error(open_result.error)) {
		fun_console_write_line("Failed to open file: ");
		fun_console_write_line(file_path);
		fun_memory_free(&buffer);
		return 1;
	}

	FileStream *stream = (FileStream *)open_result.state;

	LogStats stats = { 0 };
	char carry_over[8192] = { 0 };
	uint64_t carry_len = 0;

	while (!fun_stream_is_end_of_stream(stream)) {
		uint64_t bytes_read = 0;
		AsyncResult read_result = fun_stream_read(stream, &bytes_read);
		fun_async_await(&read_result, -1);

		if (fun_error_is_error(read_result.error)) {
			break;
		}

		if (bytes_read > 0) {
			parse_chunk((char *)buffer, bytes_read, &stats, carry_over,
						&carry_len, sizeof(carry_over));
		}
	}

	if (carry_len > 0) {
		carry_over[carry_len] = '\0';
		parse_log_line(carry_over, &stats);
	}

	fun_stream_destroy(stream);
	fun_memory_free(&buffer);

	char output[512];

	StringTemplateParam params[] = {
		{ "info", { .intValue = stats.info } },
		{ "warn", { .intValue = stats.warn } },
		{ "error", { .intValue = stats.error } },
		{ "debug", { .intValue = stats.debug } },
		{ "trace", { .intValue = stats.trace } },
		{ "total", { .intValue = stats.total_lines } },
		{ "malformed", { .intValue = stats.malformed_lines } }
	};

	fun_console_write_line("=== Log Analyzer Results ===");
	fun_console_write_line("");

	String template_levels = "Log Levels:\n"
							 "  INFO:    #{info}\n"
							 "  WARN:    #{warn}\n"
							 "  ERROR:   #{error}\n"
							 "  DEBUG:   #{debug}\n"
							 "  TRACE:   #{trace}";

	fun_string_template(template_levels, params, 7, output, sizeof(output));
	fun_console_write_line(output);

	if (stats.unknown > 0) {
		StringTemplateParam unknown_param = { "unknown",
											  { .intValue = stats.unknown } };
		String template_unknown = "  UNKNOWN: #{unknown}";
		fun_string_template(template_unknown, &unknown_param, 1, output,
							sizeof(output));
		fun_console_write_line(output);
	}

	String template_stats = "\nStatistics:\n"
							"  Total:     #{total}";

	fun_string_template(template_stats, params, 7, output, sizeof(output));
	fun_console_write_line(output);

	if (stats.malformed_lines > 0) {
		String template_malformed = "  Malformed: #{malformed}";
		fun_string_template(template_malformed, params, 7, output,
							sizeof(output));
		fun_console_write_line(output);
	}

	return 0;
}
