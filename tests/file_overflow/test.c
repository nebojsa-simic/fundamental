#include "fundamental/console/console.h"
#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"
#include "overflow_check.h"
#include <stdint.h>
#include <stdbool.h>

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

static bool test_overflow_add_no_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_add(100, 200, &result);
	return ok && (result == 300);
}

static bool test_overflow_add_with_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_add(UINT64_MAX, 1, &result);
	(void)result;
	return !ok;
}

static bool test_overflow_sub_no_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_sub(200, 100, &result);
	return ok && (result == 100);
}

static bool test_overflow_sub_with_underflow(void)
{
	uint64_t result;
	bool ok = check_overflow_sub(50, 100, &result);
	(void)result;
	return !ok;
}

static bool test_overflow_mul_no_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_mul(0x100000000ULL, 2, &result);
	return ok && (result == 0x200000000ULL);
}

static bool test_overflow_mul_with_overflow(void)
{
	uint64_t result;
	bool ok = check_overflow_mul(0x8000000000000000ULL, 2, &result);
	(void)result;
	return !ok;
}

static bool test_read_with_offset_overflow(void)
{
	const char *file_path = "test_overflow_read.txt";

	MemoryResult mem = fun_memory_allocate(1024);
	if (fun_error_is_error(mem.error)) {
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
	return success;
}

static bool test_write_with_offset_overflow(void)
{
	const char *file_path = "test_overflow_write.txt";

	MemoryResult mem = fun_memory_allocate(512);
	if (fun_error_is_error(mem.error)) {
		return false;
	}
	fun_memory_fill(mem.value, 512, 'X');

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
	return success;
}

static bool test_append_with_offset_overflow(void)
{
	const char *file_path = "test_overflow_append.txt";

	MemoryResult mem = fun_memory_allocate(512);
	if (fun_error_is_error(mem.error)) {
		return false;
	}
	fun_memory_fill(mem.value, 512, 'A');

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
	return success;
}

int main(void)
{
	fun_console_write_line("Running file overflow tests:");

	if (!test_overflow_add_no_overflow()) {
		fun_console_write_line("overflow_check add (valid) test failed");
		return 1;
	}
	fun_console_write_line("✓ overflow_check_add_no_overflow passed");

	if (!test_overflow_add_with_overflow()) {
		fun_console_write_line("overflow_check add (overflow) test failed");
		return 1;
	}
	fun_console_write_line("✓ overflow_check_add_with_overflow passed");

	if (!test_overflow_sub_no_overflow()) {
		fun_console_write_line("overflow_check sub (valid) test failed");
		return 1;
	}
	fun_console_write_line("✓ overflow_check_sub_no_overflow passed");

	if (!test_overflow_sub_with_underflow()) {
		fun_console_write_line("overflow_check sub (underflow) test failed");
		return 1;
	}
	fun_console_write_line("✓ overflow_check_sub_with_underflow passed");

	if (!test_overflow_mul_no_overflow()) {
		fun_console_write_line("overflow_check mul (valid) test failed");
		return 1;
	}
	fun_console_write_line("✓ overflow_check_mul_no_overflow passed");

	if (!test_overflow_mul_with_overflow()) {
		fun_console_write_line("overflow_check mul (overflow) test failed");
		return 1;
	}
	fun_console_write_line("✓ overflow_check_mul_with_overflow passed");

	if (!test_read_with_offset_overflow()) {
		fun_console_write_line("Read with offset overflow test failed");
		return 1;
	}
	fun_console_write_line("✓ test_read_with_offset_overflow passed");

	if (!test_write_with_offset_overflow()) {
		fun_console_write_line("Write with offset overflow test failed");
		return 1;
	}
	fun_console_write_line("✓ test_write_with_offset_overflow passed");

	if (!test_append_with_offset_overflow()) {
		fun_console_write_line("Append with offset overflow test failed");
		return 1;
	}
	fun_console_write_line("✓ test_append_with_offset_overflow passed");

	fun_console_write_line("All file overflow tests passed!");
	return 0;
}
