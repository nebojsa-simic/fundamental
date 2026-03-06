#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/file/file.h"
#include "../../include/memory/memory.h"
#include "../../include/async/async.h"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

bool test_fun_append_memory_to_file_basic(void)
{
	// Clean up any existing test file
	remove("test_append_output.txt");

	// Test appending to a new file
	const char *new_data = "New data";
	size_t data_size = strlen(new_data);

	// Allocate memory to hold the data for appending
	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	memcpy(mem_result.value, new_data, data_size);

	Append params = { .file_path = "test_append_output.txt",
					  .input = mem_result.value,
					  .bytes_to_append = data_size,
					  .mode = FILE_MODE_AUTO };

	AsyncResult append_result = fun_append_memory_to_file(params);
	fun_async_await(&append_result);

	bool success = (append_result.status == ASYNC_COMPLETED) &&
				   (fun_error_is_ok(append_result.error));

	// Verify file contents
	if (success) {
		FILE *fp = fopen("test_append_output.txt", "rb");
		if (fp) {
			char read_buffer[256] = { 0 };
			size_t read_count = fread(read_buffer, 1, data_size, fp);
			fclose(fp);
			if (read_count != data_size ||
				memcmp(read_buffer, new_data, data_size) != 0) {
				success = false;
			}
		} else {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);

	// Clean up
	remove("test_append_output.txt");

	if (success) {
		printf("✓ fun_append_memory_to_file_basic passed\n");
	}
	return success;
}

bool test_fun_append_memory_to_file_to_existing(void)
{
	// Create a file with initial content
	FILE *fp = fopen("test_append_existing.txt", "wb");
	if (!fp) {
		return false;
	}
	fwrite("Initial ", 1, 8, fp);
	fclose(fp);

	// Append additional content
	const char *append_data = "data";
	size_t data_size = strlen(append_data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		remove("test_append_existing.txt");
		return false;
	}
	memcpy(mem_result.value, append_data, data_size);

	Append params = { .file_path = "test_append_existing.txt",
					  .input = mem_result.value,
					  .bytes_to_append = data_size,
					  .mode = FILE_MODE_AUTO };

	AsyncResult append_result = fun_append_memory_to_file(params);
	fun_async_await(&append_result);

	bool success = (fun_error_is_ok(append_result.error));

	// Verify file has both initial and appended content
	if (success) {
		FILE *verify_fp = fopen("test_append_existing.txt", "rb");
		if (verify_fp) {
			char read_buffer[256] = { 0 };
			fseek(verify_fp, 0, SEEK_END);
			long file_size = ftell(verify_fp);
			if (file_size == 12) { // "Initial data" = 8 + 4
				fseek(verify_fp, 0, SEEK_SET);
				fread(read_buffer, 1, file_size, verify_fp);
				if (memcmp(read_buffer, "Initial data", 12) != 0) {
					success = false;
				}
			} else {
				success = false;
			}
			fclose(verify_fp);
		} else {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);
	remove("test_append_existing.txt");

	if (success) {
		printf("✓ fun_append_memory_to_file_to_existing passed\n");
	}
	return success;
}

bool test_fun_append_memory_to_file_large_data(void)
{
	// Clean up any existing test file
	remove("test_large_append.txt");

	// Create large dataset to append
	const size_t data_size = 1024; // 1KB
	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}

	// Fill with pattern
	for (size_t i = 0; i < data_size; i++) {
		((char *)mem_result.value)[i] = (char)(i % 256);
	}

	Append params = { .file_path = "test_large_append.txt",
					  .input = mem_result.value,
					  .bytes_to_append = data_size,
					  .mode = FILE_MODE_AUTO };

	AsyncResult append_result = fun_append_memory_to_file(params);
	fun_async_await(&append_result);

	bool success = (append_result.status == ASYNC_COMPLETED) &&
				   (fun_error_is_ok(append_result.error));

	// Verify content
	if (success) {
		FILE *fp = fopen("test_large_append.txt", "rb");
		if (fp) {
			char read_buffer[data_size];
			size_t read_count = fread(read_buffer, 1, data_size, fp);
			fclose(fp);
			if (read_count != data_size ||
				memcmp(read_buffer, mem_result.value, data_size) != 0) {
				success = false;
			}
		} else {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);
	remove("test_large_append.txt");

	if (success) {
		printf("✓ fun_append_memory_to_file_large_data passed\n");
	}
	return success;
}

int main()
{
	printf("Running file append module tests:\n");

	if (!test_fun_append_memory_to_file_basic()) {
		printf("Basic append test failed\n");
		return 1;
	}

	if (!test_fun_append_memory_to_file_to_existing()) {
		printf("Append to existing file test failed\n");
		return 1;
	}

	if (!test_fun_append_memory_to_file_large_data()) {
		printf("Large data append test failed\n");
		return 1;
	}

	printf("All file append tests passed!\n");
	return 0;
}