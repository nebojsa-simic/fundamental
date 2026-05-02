// Log Analyzer Demo - Minimal Working Version

#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/async/async.h"
#include "fundamental/hashmap/hashmap.h"

DEFINE_HASHMAP_STRING_KEY(int64_t)

int main(int argc, char **argv)
{
	if (argc < 2) {
		fun_console_write_line("Usage: demo.exe <logfile>");
		return 1;
	}

	String file_path = argv[1];
	fun_console_write_line("Opening file: ");
	fun_console_write_line(file_path);

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
		fun_console_write_line("Failed to read file, error code: ");
		char err_buf[32];
		fun_string_from_int(read_result.error.code, 10, err_buf,
							sizeof(err_buf));
		fun_console_write_line(err_buf);
		fun_memory_free(&buffer);
		return 1;
	}

	Stringint64_tHashMapResult level_counts_result =
		fun_hashmap_String_int64_t_create(16);
	if (fun_error_is_error(level_counts_result.error)) {
		fun_console_write_line("Failed to create hashmap");
		fun_memory_free(&buffer);
		return 1;
	}
	Stringint64_tHashMap level_counts = level_counts_result.value;

	char *levels[] = { "INFO", "WARN", "ERROR", "DEBUG", "TRACE" };
	for (int i = 0; i < 5; i++) {
		int64_t zero = 0;
		fun_hashmap_String_int64_t_put(&level_counts, levels[i], zero);
	}

	char *content = (char *)buffer;
	int64_t total_lines = 0;

	char *line_start = content;
	for (int i = 0; i < 65536 && content[i] != '\0'; i++) {
		if (content[i] == '\n') {
			content[i] = '\0';
			total_lines++;

			for (int j = 0; j < 5; j++) {
				if (fun_string_index_of(line_start, levels[j], 0) >= 0) {
					int64_t count = fun_hashmap_String_int64_t_get(
						&level_counts, levels[j]);
					count++;
					fun_hashmap_String_int64_t_put(&level_counts, levels[j],
												   count);
					break;
				}
			}
			line_start = &content[i + 1];
		}
	}

	fun_console_write_line("=== Log Analyzer Results ===");
	char num_buf[32];

	for (int i = 0; i < 5; i++) {
		int64_t count =
			fun_hashmap_String_int64_t_get(&level_counts, levels[i]);
		fun_console_write(levels[i]);
		fun_console_write(": ");
		fun_string_from_int(count, 10, num_buf, sizeof(num_buf));
		fun_console_write_line(num_buf);
	}

	fun_console_write_line("");
	fun_string_from_int(total_lines, 10, num_buf, sizeof(num_buf));
	fun_console_write("Total lines: ");
	fun_console_write_line(num_buf);

	fun_hashmap_String_int64_t_destroy(&level_counts);
	fun_memory_free(&buffer);

	return 0;
}
