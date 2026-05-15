#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "../../include/fundamental/file/file.h"
#include "../../include/fundamental/memory/memory.h"
#include "../../include/fundamental/error/error.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"
#define TEST_FILE_PATH "./test_permission.tmp"
#define TEST_READ_ONLY_PATH "./test_readonly.tmp"

static void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

static void delete_test_file(const char *path)
{
	chmod(path, 0666);
	unlink(path);
}

static void test_read_nonexistent_file(void)
{
	delete_test_file(TEST_FILE_PATH);

	MemoryResult mem_res = fun_memory_allocate(1024);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = 1024,
					.offset = 0,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	assert(read_res.error.code != 0);

	fun_memory_free(&buffer);
	print_test_result("read_nonexistent_file");
}

static void test_read_file_no_permission(void)
{
	delete_test_file(TEST_FILE_PATH);
	int fd = open(TEST_FILE_PATH, O_CREAT | O_WRONLY, 0000);
	assert(fd >= 0);
	close(fd);

	MemoryResult mem_res = fun_memory_allocate(1024);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = 1024,
					.offset = 0,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	if (fun_error_is_ok(read_res.error))
		fun_async_await(&read_res, -1);

	assert(read_res.error.code != 0);

	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("read_file_no_permission");
}

static void test_write_to_readonly_file(void)
{
	delete_test_file(TEST_READ_ONLY_PATH);
	int fd = open(TEST_READ_ONLY_PATH, O_CREAT | O_WRONLY, 0444);
	assert(fd >= 0);
	close(fd);

	const char *data = "Test write";
	MemoryResult mem_res = fun_memory_allocate(strlen(data));
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	memcpy(buffer, data, strlen(data));

	Write params = { .file_path = TEST_READ_ONLY_PATH,
					 .input = buffer,
					 .bytes_to_write = strlen(data),
					 .offset = 0,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res, -1);

	assert(write_res.error.code != 0);

	fun_memory_free(&buffer);
	delete_test_file(TEST_READ_ONLY_PATH);
	print_test_result("write_to_readonly_file");
}

static void test_write_to_directory(void)
{
	Write params = { .file_path = "/tmp",
					 .input = NULL,
					 .bytes_to_write = 0,
					 .offset = 0,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_memory_to_file(params);
	if (fun_error_is_ok(write_res.error))
		fun_async_await(&write_res, -1);

	assert(write_res.error.code != 0);
	print_test_result("write_to_directory");
}

static void test_read_directory(void)
{
	MemoryResult mem_res = fun_memory_allocate(1024);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = "/tmp",
					.output = buffer,
					.bytes_to_read = 1024,
					.offset = 0,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	if (fun_error_is_ok(read_res.error))
		fun_async_await(&read_res, -1);

	assert(read_res.error.code != 0);

	fun_memory_free(&buffer);
	print_test_result("read_directory");
}

int main(void)
{
	printf("Running file permission tests:\n");

	test_read_nonexistent_file();
	test_read_file_no_permission();
	test_write_to_readonly_file();
	test_write_to_directory();
	test_read_directory();

	printf("All file permission tests passed!\n");
	return 0;
}
