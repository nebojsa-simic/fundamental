#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"
#include "overflow_check.h"
#include <stdint.h>
#include <stdbool.h>

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result)    assert(result.error.code != 0)

static bool
test_overflow_add_no_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_add(100, 200, &result);
	return ok && (result == 300);
}

static bool
test_overflow_add_with_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_add(UINT64_MAX, 1, &result);
	(void)result;
	return !ok;
}

static bool
test_overflow_sub_no_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_sub(200, 100, &result);
	return ok && (result == 100);
}

static bool
test_overflow_sub_with_underflow(void)
{
	uint64_t result;
	bool ok = check_overflow_sub(50, 100, &result);
	(void)result;
	return !ok;
}

static bool
test_overflow_mul_no_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_mul(0x100000000ULL, 2, &result);
	return ok && (result == 0x200000000ULL);
}

static bool
test_overflow_mul_with_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_mul(0x8000000000000000ULL, 2, &result);
	(void)result;
	return !ok;
}

static bool
test_read_with_offset_overflow(void)
{
	const char *file_path = "test_overflow_read.txt";

	FILE *fp = fopen(file_path, "w");
	if (!fp)
		return false;
	fprintf(fp, "data");
	fclose(fp);

	MemoryResult mem = fun_memory_allocate(1024);
	if (fun_error_is_error(mem.error)) {
		remove(file_path);
		return false;
	}

	Read params = { .file_path = file_path,
			.output = mem.value,
			.bytes_to_read = 1024,
			.offset = UINT64_MAX,
			.mode = FILE_MODE_AUTO,
			.adaptive = NULL };

	AsyncResult result = fun_read_file_in_memory(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_ERROR) &&
		(fun_error_is_error(result.error));

	fun_memory_free(&mem.value);
	remove(file_path);
	return success;
}

static bool
test_write_with_offset_overflow(void)
{
	const char *file_path = "test_overflow_write.txt";
	remove(file_path);

	MemoryResult mem = fun_memory_allocate(512);
	if (fun_error_is_error(mem.error)) {
		return false;
	}
	memset(mem.value, 'X', 512);

	Write params = { .file_path = file_path,
			.input = mem.value,
			.bytes_to_write = 512,
			.offset = UINT64_MAX,
			.mode = FILE_MODE_AUTO,
			.durability_mode = FILE_DURABILITY_ASYNC,
			.adaptive = NULL };

	AsyncResult result = fun_write_memory_to_file(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_ERROR) &&
		(fun_error_is_error(result.error));

	fun_memory_free(&mem.value);
	remove(file_path);
	return success;
}

static bool
test_append_with_offset_overflow(void)
{
	const char *file_path = "test_overflow_append.txt";
	remove(file_path);

	MemoryResult mem = fun_memory_allocate(512);
	if (fun_error_is_error(mem.error)) {
		return false;
	}
	memset(mem.value, 'A', 512);

	Append params = { .file_path = file_path,
			.input = mem.value,
			.bytes_to_append = UINT64_MAX,
			.mode = FILE_MODE_AUTO,
			.durability_mode = FILE_DURABILITY_ASYNC,
			.adaptive = NULL };

	AsyncResult result = fun_append_memory_to_file(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_ERROR) &&
		(fun_error_is_error(result.error));

	fun_memory_free(&mem.value);
	remove(file_path);
	return success;
}

int main(void)
{
	printf("Running file overflow tests:\n");

	if (!test_overflow_add_no_overflow()) {
		printf("overflow_check add (valid) test failed\n");
		return 1;
	}
	printf("✓ overflow_check_add_no_overflow passed\n");

	if (!test_overflow_add_with_overflow()) {
		printf("overflow_check add (overflow) test failed\n");
		return 1;
	}
	printf("✓ overflow_check_add_with_overflow passed\n");

	if (!test_overflow_sub_no_overflow()) {
		printf("overflow_check sub (valid) test failed\n");
		return 1;
	}
	printf("✓ overflow_check_sub_no_overflow passed\n");

	if (!test_overflow_sub_with_underflow()) {
		printf("overflow_check sub (underflow) test failed\n");
		return 1;
	}
	printf("✓ overflow_check_sub_with_underflow passed\n");

	if (!test_overflow_mul_no_overflow()) {
		printf("overflow_check mul (valid) test failed\n");
		return 1;
	}
	printf("✓ overflow_check_mul_no_overflow passed\n");

	if (!test_overflow_mul_with_overflow()) {
		printf("overflow_check mul (overflow) test failed\n");
		return 1;
	}
	printf("✓ overflow_check_mul_with_overflow passed\n");

	if (!test_read_with_offset_overflow()) {
		printf("Read with offset overflow test failed\n");
		return 1;
	}
	printf("✓ test_read_with_offset_overflow passed\n");

	if (!test_write_with_offset_overflow()) {
		printf("Write with offset overflow test failed\n");
		return 1;
	}
	printf("✓ test_write_with_offset_overflow passed\n");

	if (!test_append_with_offset_overflow()) {
		printf("Append with offset overflow test failed\n");
		return 1;
	}
	printf("✓ test_append_with_offset_overflow passed\n");

	printf("All file overflow tests passed!\n");
	return 0;
}
