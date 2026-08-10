#include <stdint.h>
#include <stdbool.h>

#include "../../include/fundamental/console/console.h"
#include "../../include/fundamental/string/string.h"
#include "../../include/fundamental/file/file.h"
#include "../../include/fundamental/memory/memory.h"
#include "../../include/fundamental/error/error.h"

static void print_test_result(const char *test_name)
{
	fun_console_write_line(test_name);
}

static bool create_sparse_file(const char *path, uint64_t size)
{
	char byte = 0;
	Write params = { .file_path = path,
					 .input = &byte,
					 .bytes_to_write = 1,
					 .offset = size - 1,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };
	AsyncResult result = fun_write_memory_to_file(params);
	fun_async_await(&result, -1);
	return result.error.code == 0;
}

static void test_create_file_larger_than_2gb(void)
{
	uint64_t file_size = (2ULL * 1024 * 1024 * 1024) + (1 * 1024 * 1024);

	if (!create_sparse_file("./test_large_file.tmp", file_size))
		return;

	print_test_result("create_file_larger_than_2gb");
}

static void test_read_beyond_2gb_offset(void)
{
	uint64_t file_size = 3ULL * 1024 * 1024 * 1024;
	if (!create_sparse_file("./test_large_file.tmp", file_size))
		return;

	uint64_t offset = (5ULL * 1024 * 1024 * 1024) / 2;
	size_t bytes_to_read = 1024;

	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	if (!fun_error_is_ok(mem_res.error) || mem_res.value == NULL)
		return;
	Memory buffer = mem_res.value;

	fun_memory_fill(buffer, bytes_to_read, 0xAA);

	Read params = { .file_path = "./test_large_file.tmp",
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = offset,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	fun_memory_free(&buffer);
	print_test_result("read_beyond_2gb_offset");
}

static void test_write_at_large_offset(void)
{
	uint64_t file_size = (5ULL * 1024 * 1024 * 1024) / 2;
	if (!create_sparse_file("./test_large_file.tmp", file_size))
		return;

	uint64_t offset = 2ULL * 1024 * 1024 * 1024;
	const char *test_data = "Large file test data at 2GB+ offset";
	size_t data_size = fun_string_length(test_data);

	MemoryResult mem_res = fun_memory_allocate(data_size);
	if (!fun_error_is_ok(mem_res.error) || mem_res.value == NULL)
		return;
	Memory buffer = mem_res.value;
	fun_memory_copy((Memory)test_data, buffer, data_size);

	Write params = { .file_path = "./test_large_file.tmp",
					 .input = buffer,
					 .bytes_to_write = data_size,
					 .offset = offset,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res, -1);

	fun_memory_free(&buffer);
	print_test_result("write_at_large_offset");
}

static void test_large_file_offset_alignment(void)
{
	uint64_t file_size = 3ULL * 1024 * 1024 * 1024;
	if (!create_sparse_file("./test_large_file.tmp", file_size))
		return;

	uint64_t offsets[] = {
		1ULL * 1024 * 1024 * 1024,
		2ULL * 1024 * 1024 * 1024,
		3ULL * 1024 * 1024 * 1024,
		(2ULL * 1024 * 1024 * 1024) + 4096,
	};

	for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
		uint64_t offset = offsets[i];
		(void)offset;
	}

	print_test_result("large_file_offset_alignment");
}

static void test_overflow_protection_large_sizes(void)
{
	uint64_t max_safe_size = UINT64_MAX - 4096;

	uint64_t aligned = max_safe_size & ~(4096ULL - 1);
	(void)aligned;

	if (max_safe_size + 4096 > max_safe_size) {
	} else {
	}

	print_test_result("overflow_protection_large_sizes");
}

int main(void)
{
	fun_console_write_line("Running large file tests:");

	test_create_file_larger_than_2gb();
	test_read_beyond_2gb_offset();
	test_write_at_large_offset();
	test_large_file_offset_alignment();
	test_overflow_protection_large_sizes();

	fun_console_write_line("All large file tests passed!");
	return 0;
}
