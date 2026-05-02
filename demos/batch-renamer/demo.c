// Batch Renamer Demo - Fundamental Library
// Minimal implementation: List files, preview rename with find/replace

#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"
#include "fundamental/filesystem/filesystem.h"
#include "fundamental/filesystem/path.h"

int main(int argc, char **argv)
{
	if (argc < 4) {
		fun_console_write_line("=== Batch Renamer Demo ===");
		fun_console_write_line(
			"Usage: demo.exe <directory> <find> <replace> [--apply]");
		fun_console_write_line("");
		fun_console_write_line("Examples:");
		fun_console_write_line(
			"  demo.exe . txt bak     # Rename .txt to .bak");
		fun_console_write_line(
			"  demo.exe . old new     # Replace 'old' with 'new'");
		fun_console_write_line(
			"  demo.exe . txt bak --apply  # Actually rename files");
		return 1;
	}

	String dir_path = argv[1];
	String find_str = argv[2];
	String replace_str = argv[3];
	int apply_mode = (argc > 4 && fun_string_compare(argv[4], "--apply") == 0);

	fun_console_write_line("=== Batch Renamer Demo ===");
	if (apply_mode) {
		fun_console_write_line("Mode: APPLY (files will be renamed)");
	} else {
		fun_console_write_line("Mode: DRY-RUN (preview only)");
	}
	fun_console_write_line("");

	Path root_path;
	const char *root_components[16];
	root_path.components = root_components;
	root_path.is_absolute = false;
	root_path.count = 1;
	root_components[0] = dir_path;

	boolResult exists = fun_directory_exists(root_path);
	if (fun_error_is_error(exists.error) || !exists.value) {
		fun_console_write_line("Directory does not exist: ");
		fun_console_write_line(dir_path);
		return 1;
	}

	MemoryResult list_result = fun_memory_allocate(8192);
	if (fun_error_is_error(list_result.error)) {
		fun_console_write_line("Failed to allocate buffer");
		return 1;
	}
	Memory list_buffer = list_result.value;

	ErrorResult list_dir_result =
		fun_filesystem_list_directory(root_path, list_buffer);
	if (fun_error_is_error(list_dir_result)) {
		fun_console_write_line("Failed to list directory");
		fun_memory_free(&list_buffer);
		return 1;
	}

	char *list_content = (char *)list_buffer;

	fun_console_write_line("Files to rename:");
	fun_console_write_line("");

	char *line_start = list_content;
	int file_count = 0;

	for (int i = 0; i < 8192 && list_content[i] != '\0'; i++) {
		if (list_content[i] == '\n') {
			list_content[i] = '\0';

			if (line_start[0] == 'F' && line_start[1] == '\t') {
				char *filename = line_start + 2;
				StringLength fname_len = fun_string_length(filename);

				if (fun_string_index_of(filename, find_str, 0) >= 0) {
					file_count++;

					char new_name[256];
					new_name[0] = '\0';

					StringLength find_len = fun_string_length(find_str);
					StringLength replace_len = fun_string_length(replace_str);

					StringPosition pos = 0;
					StringPosition last_pos = 0;

					while ((pos = fun_string_index_of(filename, find_str,
													  last_pos)) >= 0) {
						char temp[256];

						if (pos > last_pos) {
							fun_string_substring(filename, last_pos,
												 pos - last_pos, temp,
												 sizeof(temp));
							fun_string_join(new_name, temp, new_name,
											sizeof(new_name));
						}

						fun_string_join(new_name, replace_str, new_name,
										sizeof(new_name));
						last_pos = pos + find_len;
					}

					if (last_pos < fname_len) {
						char temp[256];
						fun_string_substring(filename, last_pos,
											 fname_len - last_pos, temp,
											 sizeof(temp));
						fun_string_join(new_name, temp, new_name,
										sizeof(new_name));
					}

					fun_console_write("  ");
					fun_console_write(filename);
					fun_console_write(" -> ");
					fun_console_write_line(new_name);
				}
			}
			line_start = &list_content[i + 1];
		}
	}

	fun_console_write_line("");
	fun_console_write_line("Summary:");

	char num_buf[32];
	fun_string_from_int(file_count, 10, num_buf, sizeof(num_buf));
	fun_console_write("  Files matching pattern: ");
	fun_console_write_line(num_buf);

	if (apply_mode) {
		fun_console_write_line("  TODO: Implement actual renaming");
	} else {
		fun_console_write_line("  Run with --apply to rename files");
	}

	fun_memory_free(&list_buffer);

	return 0;
}
