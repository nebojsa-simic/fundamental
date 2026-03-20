#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define TEST_FILENAME "test_write_file.tmp"

static bool create_empty_test_file(const char *path)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return false;
	close(fd);
	return true;
}

static bool test_fun_write_memory_to_file_basic(void)
{
	const char *content = "Hello, World!";
	size_t len = strlen(content);

	if (!create_empty_test_file(TEST_FILENAME)) {
		return false;
	}

	MemoryResult buf = fun_memory_allocate(len);
	if (fun_error_is_error(buf.error)) {
		unlink(TEST_FILENAME);
		return false;
	}
	memcpy(buf.value, content, len);

	Write params = { .file_path = TEST_FILENAME,
					 .input = buf.value,
					 .bytes_to_write = len,
					 .offset = 0,
					 .mode = FILE_MODE_AUTO };

	AsyncResult result = fun_write_memory_to_file(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_COMPLETED) &&
				   fun_error_is_ok(result.error);

	if (success) {
		FILE *fp = fopen(TEST_FILENAME, "rb");
		if (fp) {
			char read_buf[256] = { 0 };
			size_t rd = fread(read_buf, 1, len, fp);
			fclose(fp);
			success = (rd == len && memcmp(read_buf, content, len) == 0);
		} else {
			success = false;
		}
	}

	fun_memory_free(&buf.value);
	unlink(TEST_FILENAME);

	if (success) {
		printf("%s test_fun_write_memory_to_file_basic\n", GREEN_CHECK);
	}
	return success;
}

static bool test_fun_write_memory_to_file_offset(void)
{
	const char *content = "WORLD";
	size_t len = strlen(content);

	if (!create_empty_test_file(TEST_FILENAME)) {
		return false;
	}

	MemoryResult buf = fun_memory_allocate(len);
	if (fun_error_is_error(buf.error)) {
		unlink(TEST_FILENAME);
		return false;
	}
	memcpy(buf.value, content, len);

	Write params = { .file_path = TEST_FILENAME,
					 .input = buf.value,
					 .bytes_to_write = len,
					 .offset = 2,
					 .mode = FILE_MODE_AUTO };

	AsyncResult result = fun_write_memory_to_file(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_COMPLETED) &&
				   fun_error_is_ok(result.error);

	if (success) {
		FILE *fp = fopen(TEST_FILENAME, "rb");
		if (fp) {
			char read_buf[256] = { 0 };
			size_t rd = fread(read_buf, 1, len + 2, fp);
			fclose(fp);
			success =
				(rd == len + 2 && memcmp(read_buf + 2, content, len) == 0);
		} else {
			success = false;
		}
	}

	fun_memory_free(&buf.value);
	unlink(TEST_FILENAME);

	if (success) {
		printf("%s test_fun_write_memory_to_file_offset\n", GREEN_CHECK);
	}
	return success;
}

int main(void)
{
	printf("Running file write module tests:\n");

	int failures = 0;
	if (!test_fun_write_memory_to_file_basic())
		failures++;
	if (!test_fun_write_memory_to_file_offset())
		failures++;

	if (failures == 0) {
		printf("All file write tests passed!\n");
		return 0;
	}
	printf("%d file write tests failed\n", failures);
	return 1;
}
