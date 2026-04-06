#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "../../../include/fundamental/file/file.h"
#include "../../../include/fundamental/memory/memory.h"
#include "../../../include/fundamental/error/error.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"
#define TEST_FILE_PATH_UNIX "./test_permission.tmp"
#define TEST_READ_ONLY_PATH_UNIX "./test_readonly.tmp"
#define TEST_DIR_PATH_UNIX "./test_no_access_dir"

#ifdef _WIN32
#define TEST_FILE_PATH ".\\test_permission.tmp"
#define TEST_READ_ONLY_PATH ".\\test_readonly.tmp"
#define TEST_DIR_PATH ".\\test_no_access_dir"
#else
#define TEST_FILE_PATH TEST_FILE_PATH_UNIX
#define TEST_READ_ONLY_PATH TEST_READ_ONLY_PATH_UNIX
#define TEST_DIR_PATH TEST_DIR_PATH_UNIX
#endif

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void delete_test_file(const char *path)
{
#ifdef _WIN32
	DeleteFileA(path);
#else
	unlink(path);
#endif
}

#ifdef _WIN32
#include <windows.h>
// Windows implementation
void test_read_nonexistent_file(void)
{
	delete_test_file(TEST_FILE_PATH); // Ensure it doesn't exist

	MemoryResult mem_res = fun_memory_allocate(1024);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = STRING_LITERAL(TEST_FILE_PATH),
					.output = buffer,
					.bytes_to_read = 1024,
					.offset = 0,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	assert(read_res.error.code == ERROR_CODE_PATH_INVALID ||
		   read_res.error.code == ERROR_CODE_PERMISSION_DENIED);

	fun_memory_free(&buffer);
	print_test_result("read_nonexistent_file");
}

void test_write_to_readonly_file(void)
{
	// Create file
	HANDLE hFile = CreateFileA(TEST_READ_ONLY_PATH, GENERIC_WRITE, 0, NULL,
							   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(hFile != INVALID_HANDLE_VALUE);
	CloseHandle(hFile);

	// Make read-only
	SetFileAttributesA(TEST_READ_ONLY_PATH, FILE_ATTRIBUTE_READONLY);

	const char *data = "Test write";
	MemoryResult mem_res = fun_memory_allocate(strlen(data));
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	memcpy(buffer, data, strlen(data));

	Write params = { .file_path = STRING_LITERAL(TEST_READ_ONLY_PATH),
					 .input = buffer,
					 .bytes_to_write = strlen(data),
					 .offset = 0,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_file_in_memory(params);
	fun_async_await(&write_res, -1);

	assert(write_res.error.code == ERROR_CODE_PERMISSION_DENIED ||
		   write_res.error.code != 0); // Should fail

	fun_memory_free(&buffer);
	SetFileAttributesA(TEST_READ_ONLY_PATH, FILE_ATTRIBUTE_NORMAL);
	delete_test_file(TEST_READ_ONLY_PATH);
	print_test_result("write_to_readonly_file");
}

#else
// Linux/Unix implementation

void test_read_nonexistent_file(void)
{
	delete_test_file(TEST_FILE_PATH); // Ensure it doesn't exist

	MemoryResult mem_res = fun_memory_allocate(1024);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = STRING_LITERAL(TEST_FILE_PATH),
					.output = buffer,
					.bytes_to_read = 1024,
					.offset = 0,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	assert(read_res.error.code != 0); // Should fail

	fun_memory_free(&buffer);
	print_test_result("read_nonexistent_file");
}

void test_read_file_no_permission(void)
{
	// Create a file with no read permissions
	int fd = open(TEST_FILE_PATH, O_CREAT | O_WRONLY, 0000);
	assert(fd >= 0);
	close(fd);

	MemoryResult mem_res = fun_memory_allocate(1024);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = STRING_LITERAL(TEST_FILE_PATH),
					.output = buffer,
					.bytes_to_read = 1024,
					.offset = 0,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	assert(read_res.error.code == ERROR_CODE_PERMISSION_DENIED ||
		   read_res.error.code != 0); // Should fail

	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("read_file_no_permission");
}

void test_write_to_readonly_file(void)
{
	// Create read-only file
	int fd = open(TEST_READ_ONLY_PATH, O_CREAT | O_WRONLY, 0444);
	assert(fd >= 0);
	close(fd);

	const char *data = "Test write";
	MemoryResult mem_res = fun_memory_allocate(strlen(data));
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	memcpy(buffer, data, strlen(data));

	Write params = { .file_path = STRING_LITERAL(TEST_READ_ONLY_PATH),
					 .input = buffer,
					 .bytes_to_write = strlen(data),
					 .offset = 0,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_file_in_memory(params);
	fun_async_await(&write_res, -1);

	assert(write_res.error.code == ERROR_CODE_PERMISSION_DENIED ||
		   write_res.error.code != 0); // Should fail

	fun_memory_free(&buffer);
	delete_test_file(TEST_READ_ONLY_PATH);
	print_test_result("write_to_readonly_file");
}

void test_write_to_directory(void)
{
	// Try to write to a directory path
	Write params = { .file_path = STRING_LITERAL("/tmp"),
					 .input = NULL,
					 .bytes_to_write = 0,
					 .offset = 0,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_file_in_memory(params);
	fun_async_await(&write_res, -1);

	assert(write_res.error.code != 0); // Should fail
	print_test_result("write_to_directory");
}

void test_read_directory(void)
{
	// Try to read a directory as a file
	MemoryResult mem_res = fun_memory_allocate(1024);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = STRING_LITERAL("/tmp"),
					.output = buffer,
					.bytes_to_read = 1024,
					.offset = 0,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	assert(read_res.error.code != 0); // Should fail

	fun_memory_free(&buffer);
	print_test_result("read_directory");
}

#endif

int main(void)
{
	printf("Running file permission tests:\n");

	test_read_nonexistent_file();
#ifdef _WIN32
	test_write_to_readonly_file();
#else
	test_read_file_no_permission();
	test_write_to_readonly_file();
	test_write_to_directory();
	test_read_directory();
#endif

	printf("All file permission tests passed!\n");
	return 0;
}
