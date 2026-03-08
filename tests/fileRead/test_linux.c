#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "../../include/file/file.h"
#include "../../include/memory/memory.h"
#include "../../include/async/async.h"

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define TEST_FILENAME "test_read_file.tmp"

static bool create_test_file(const char *path, const char *content, size_t len)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) return false;
	if (len > 0 && write(fd, content, len) != (ssize_t)len) {
		close(fd);
		unlink(path);
		return false;
	}
	close(fd);
	return true;
}

static bool test_fun_read_file_in_memory_basic(void)
{
	const char *content = "Hello, World!";
	size_t len = strlen(content);
	
	if (!create_test_file(TEST_FILENAME, content, len)) {
		return false;
	}
	
	MemoryResult buf = fun_memory_allocate(len + 1);
	if (fun_error_is_error(buf.error)) {
		unlink(TEST_FILENAME);
		return false;
	}
	memset(buf.value, 0, len + 1);
	
	Read params = {
		.file_path = TEST_FILENAME,
		.output = buf.value,
		.bytes_to_read = len,
		.offset = 0,
		.mode = FILE_MODE_STANDARD
	};
	
	AsyncResult result = fun_read_file_in_memory(params);
	fun_async_await(&result);
	
	bool success = (result.status == ASYNC_COMPLETED) && fun_error_is_ok(result.error);
	
	if (success) {
		success = (memcmp(buf.value, content, len) == 0);
	}
	
	fun_memory_free(&buf.value);
	unlink(TEST_FILENAME);
	
	if (success) {
		printf("%s test_fun_read_file_in_memory_basic\n", GREEN_CHECK);
	}
	return success;
}

static bool test_fun_read_file_in_memory_offset(void)
{
	const char *content = "0123456789";
	size_t len = strlen(content);
	
	if (!create_test_file(TEST_FILENAME, content, len)) {
		return false;
	}
	
	char buffer[6] = {0};
	
	MemoryResult buf = fun_memory_allocate(5);
	if (fun_error_is_error(buf.error)) {
		unlink(TEST_FILENAME);
		return false;
	}
	
	Read params = {
		.file_path = TEST_FILENAME,
		.output = buf.value,
		.bytes_to_read = 5,
		.offset = 2,
		.mode = FILE_MODE_STANDARD
	};
	
	AsyncResult result = fun_read_file_in_memory(params);
	fun_async_await(&result);
	
	bool success = (result.status == ASYNC_COMPLETED) && fun_error_is_ok(result.error);
	if (success) {
		memcpy(buffer, buf.value, 5);
		success = (strcmp(buffer, "23456") == 0);
	}
	
	fun_memory_free(&buf.value);
	unlink(TEST_FILENAME);
	
	if (success) {
		printf("%s test_fun_read_file_in_memory_offset\n", GREEN_CHECK);
	}
	return success;
}

static bool test_fun_read_file_in_memory_null_pointer(void)
{
	Read params = { .file_path = NULL, .output = NULL, .bytes_to_read = 0, .offset = 0, .mode = FILE_MODE_STANDARD };
	AsyncResult result = fun_read_file_in_memory(params);
	
	bool success = (result.status == ASYNC_ERROR) && !fun_error_is_ok(result.error);
	
	if (success) {
		printf("%s test_fun_read_file_in_memory_null_pointer\n", GREEN_CHECK);
	}
	return success;
}

int main(void)
{
	printf("Running file read module tests:\n");
	
	int failures = 0;
	if (!test_fun_read_file_in_memory_basic()) failures++;
	if (!test_fun_read_file_in_memory_offset()) failures++;
	if (!test_fun_read_file_in_memory_null_pointer()) failures++;
	
	if (failures == 0) {
		printf("All file read tests passed!\n");
		return 0;
	}
	printf("%d file read tests failed\n", failures);
	return 1;
}
