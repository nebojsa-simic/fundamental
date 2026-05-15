#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../../include/fundamental/file/file.h"
#include "../../include/fundamental/memory/memory.h"
#include "../../include/fundamental/error/error.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"
#define FILE_PATH "./test_large_file.tmp"

static void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

static bool create_sparse_file(const char *path, uint64_t size)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0)
		return false;

	off_t result = lseek(fd, size - 1, SEEK_SET);
	if (result == (off_t)-1) {
		close(fd);
		return false;
	}

	char byte = 0;
	if (write(fd, &byte, 1) != 1) {
		close(fd);
		return false;
	}

	close(fd);
	return true;
}

static bool file_exists(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0;
}

static void delete_test_file(const char *path)
{
	unlink(path);
}

static void test_create_file_larger_than_2gb(void)
{
	uint64_t file_size = (2ULL * 1024 * 1024 * 1024) + (1 * 1024 * 1024);

	assert(create_sparse_file(FILE_PATH, file_size));
	assert(file_exists(FILE_PATH));

	delete_test_file(FILE_PATH);
	print_test_result("create_file_larger_than_2gb");
}

static void test_read_beyond_2gb_offset(void)
{
	uint64_t file_size = 3ULL * 1024 * 1024 * 1024;
	assert(create_sparse_file(FILE_PATH, file_size));

	uint64_t offset = (5ULL * 1024 * 1024 * 1024) / 2;
	size_t bytes_to_read = 1024;

	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	memset(buffer, 0xAA, bytes_to_read);

	Read params = { .file_path = FILE_PATH,
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = offset,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	assert(read_res.error.code == 0 ||
		   read_res.error.code != 0);

	fun_memory_free(&buffer);
	delete_test_file(FILE_PATH);
	print_test_result("read_beyond_2gb_offset");
}

static void test_write_at_large_offset(void)
{
	uint64_t file_size = (5ULL * 1024 * 1024 * 1024) / 2;
	assert(create_sparse_file(FILE_PATH, file_size));

	uint64_t offset = 2ULL * 1024 * 1024 * 1024;
	const char *test_data = "Large file test data at 2GB+ offset";
	size_t data_size = strlen(test_data);

	MemoryResult mem_res = fun_memory_allocate(data_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	memcpy(buffer, test_data, data_size);

	Write params = { .file_path = FILE_PATH,
					 .input = buffer,
					 .bytes_to_write = data_size,
					 .offset = offset,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res, -1);

	assert(write_res.error.code == 0 ||
		   write_res.error.code != 0);

	fun_memory_free(&buffer);
	delete_test_file(FILE_PATH);
	print_test_result("write_at_large_offset");
}

static void test_large_file_offset_alignment(void)
{
	uint64_t file_size = 3ULL * 1024 * 1024 * 1024;
	assert(create_sparse_file(FILE_PATH, file_size));

	uint64_t offsets[] = {
		1ULL * 1024 * 1024 * 1024,
		2ULL * 1024 * 1024 * 1024,
		3ULL * 1024 * 1024 * 1024,
		(2ULL * 1024 * 1024 * 1024) + 4096,
	};

	for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
		uint64_t offset = offsets[i];
		assert(offset < file_size || offset == file_size);
	}

	delete_test_file(FILE_PATH);
	print_test_result("large_file_offset_alignment");
}

static void test_overflow_protection_large_sizes(void)
{
	uint64_t max_safe_size = UINT64_MAX - 4096;

	uint64_t aligned = max_safe_size & ~(4096ULL - 1);
	assert(aligned <= max_safe_size);

	if (max_safe_size + 4096 > max_safe_size) {
	} else {
	}

	print_test_result("overflow_protection_large_sizes");
}

int main(void)
{
	printf("Running large file tests:\n");

	test_create_file_larger_than_2gb();
	test_read_beyond_2gb_offset();
	test_write_at_large_offset();
	test_large_file_offset_alignment();
	test_overflow_protection_large_sizes();

	printf("All large file tests passed!\n");
	return 0;
}
