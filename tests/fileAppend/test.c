#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"
#include "fundamental/console/console.h"

bool test_fun_append_memory_to_file_basic(void)
{
	const char *new_data = "New data";
	size_t data_size = fun_string_length(new_data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	fun_memory_copy((Memory)new_data, mem_result.value, data_size);

	Append params = { .file_path = "test_append_output.txt",
					  .input = mem_result.value,
					  .bytes_to_append = data_size,
					  .mode = FILE_MODE_AUTO };

	AsyncResult append_result = fun_append_memory_to_file(params);
	fun_async_await(&append_result, -1);

	bool success = (append_result.status == ASYNC_COMPLETED) &&
				   (fun_error_is_ok(append_result.error));

	if (success) {
		char read_buffer[256] = { 0 };
		Read readParams = { .file_path = "test_append_output.txt",
							.output = read_buffer,
							.bytes_to_read = data_size,
							.offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(readParams);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
			fun_memory_compare(read_buffer, new_data, data_size).value != 0) {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);

	if (success) {
		fun_console_write_line("✓ fun_append_memory_to_file_basic passed");
	}
	return success;
}

bool test_fun_append_memory_to_file_to_existing(void)
{
	const char *append_data = "data";
	size_t data_size = fun_string_length(append_data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	fun_memory_copy((Memory)append_data, mem_result.value, data_size);

	Append params = { .file_path = "test_append_existing.txt",
					  .input = mem_result.value,
					  .bytes_to_append = data_size,
					  .mode = FILE_MODE_AUTO };

	AsyncResult append_result = fun_append_memory_to_file(params);
	fun_async_await(&append_result, -1);

	bool success = (fun_error_is_ok(append_result.error));

	if (success) {
		char read_buffer[256] = { 0 };
		Read readParams = { .file_path = "test_append_existing.txt",
							.output = read_buffer,
							.bytes_to_read = 12,
							.offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(readParams);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
			fun_memory_compare(read_buffer, "Initial data", 12).value != 0) {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);

	if (success) {
		fun_console_write_line(
			"✓ fun_append_memory_to_file_to_existing passed");
	}
	return success;
}

bool test_fun_append_memory_to_file_large_data(void)
{
	const size_t data_size = 1024;
	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}

	for (size_t i = 0; i < data_size; i++) {
		((char *)mem_result.value)[i] = (char)(i % 256);
	}

	Append params = { .file_path = "test_large_append.txt",
					  .input = mem_result.value,
					  .bytes_to_append = data_size,
					  .mode = FILE_MODE_AUTO };

	AsyncResult append_result = fun_append_memory_to_file(params);
	fun_async_await(&append_result, -1);

	bool success = (append_result.status == ASYNC_COMPLETED) &&
				   (fun_error_is_ok(append_result.error));

	if (success) {
		char read_buffer[1024];
		Read readParams = { .file_path = "test_large_append.txt",
							.output = read_buffer,
							.bytes_to_read = data_size,
							.offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(readParams);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
			fun_memory_compare(read_buffer, mem_result.value, data_size).value != 0) {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);

	if (success) {
		fun_console_write_line("✓ fun_append_memory_to_file_large_data passed");
	}
	return success;
}

int main()
{
	fun_console_write_line("Running file append module tests:");

	if (!test_fun_append_memory_to_file_basic()) {
		fun_console_write_line("Basic append test failed");
		return 1;
	}

	if (!test_fun_append_memory_to_file_to_existing()) {
		fun_console_write_line("Append to existing file test failed");
		return 1;
	}

	if (!test_fun_append_memory_to_file_large_data()) {
		fun_console_write_line("Large data append test failed");
		return 1;
	}

	fun_console_write_line("All file append tests passed!");
	return 0;
}
