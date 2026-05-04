// Log Analyzer Demo - Fundamental Library
// Proper implementation: robust parsing, proper log format handling

#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/async/async.h"
#include "fundamental/hashmap/hashmap.h"

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

static void parse_buffer(char *buffer, uint64_t bytes_read, LogStats *stats)
{
	uint64_t line_start = 0;

	for (uint64_t i = 0; i < bytes_read; i++) {
		if (buffer[i] == '\n' || buffer[i] == '\r') {
			if (i > line_start) {
				buffer[i] = '\0';
				String line = &buffer[line_start];
				if (fun_string_length(line) > 0) {
					parse_log_line(line, stats);
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

	if (line_start < bytes_read && bytes_read > 0) {
		buffer[bytes_read] = '\0';
		String line = &buffer[line_start];
		if (fun_string_length(line) > 0) {
			parse_log_line(line, stats);
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

	MemoryResult buffer_result = fun_memory_allocate(65536);
	if (fun_error_is_error(buffer_result.error)) {
		fun_console_write_line("Failed to allocate buffer");
		return 1;
	}
	Memory buffer = buffer_result.value;

	Read read_params = { .file_path = file_path,
						 .output = buffer,
						 .bytes_to_read = 65536,
						 .mode = FILE_MODE_AUTO };

	AsyncResult read_result = fun_read_file_in_memory(read_params);
	fun_async_await(&read_result, -1);

	if (fun_error_is_error(read_result.error)) {
		fun_console_write_line("Failed to open file: ");
		fun_console_write_line(file_path);
		fun_memory_free(&buffer);
		return 1;
	}

	LogStats stats = { 0 };
	parse_buffer((char *)buffer, 65536, &stats);

	fun_memory_free(&buffer);

	fun_console_write_line("=== Log Analyzer Results ===");
	fun_console_write_line("");

	char num_buf[32];

	fun_console_write_line("Log Levels:");
	fun_console_write("  INFO:    ");
	fun_string_from_int(stats.info, 10, num_buf, sizeof(num_buf));
	fun_console_write_line(num_buf);

	fun_console_write("  WARN:    ");
	fun_string_from_int(stats.warn, 10, num_buf, sizeof(num_buf));
	fun_console_write_line(num_buf);

	fun_console_write("  ERROR:   ");
	fun_string_from_int(stats.error, 10, num_buf, sizeof(num_buf));
	fun_console_write_line(num_buf);

	fun_console_write("  DEBUG:   ");
	fun_string_from_int(stats.debug, 10, num_buf, sizeof(num_buf));
	fun_console_write_line(num_buf);

	fun_console_write("  TRACE:   ");
	fun_string_from_int(stats.trace, 10, num_buf, sizeof(num_buf));
	fun_console_write_line(num_buf);

	if (stats.unknown > 0) {
		fun_console_write("  UNKNOWN: ");
		fun_string_from_int(stats.unknown, 10, num_buf, sizeof(num_buf));
		fun_console_write_line(num_buf);
	}

	fun_console_write_line("");
	fun_console_write_line("Statistics:");

	fun_console_write("  Total lines:     ");
	fun_string_from_int(stats.total_lines, 10, num_buf, sizeof(num_buf));
	fun_console_write_line(num_buf);

	if (stats.malformed_lines > 0) {
		fun_console_write("  Malformed lines: ");
		fun_string_from_int(stats.malformed_lines, 10, num_buf,
							sizeof(num_buf));
		fun_console_write_line(num_buf);
	}

	return 0;
}
