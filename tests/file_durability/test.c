#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result)    assert(result.error.code != 0)

static bool
test_write_with_async_durability(void)
{
	remove("test_durability_async.txt");

	const char *data = "async test data";
	size_t data_size = strlen(data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	memcpy(mem_result.value, data, data_size);

	Write params = { .file_path = "test_durability_async.txt",
			.input = mem_result.value,
			.bytes_to_write = data_size,
			.offset = 0,
			.mode = FILE_MODE_AUTO,
			.durability_mode = FILE_DURABILITY_ASYNC,
			.adaptive = NULL };

	AsyncResult result = fun_write_memory_to_file(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_COMPLETED) &&
		(fun_error_is_ok(result.error));

	if (success) {
		FILE *fp = fopen("test_durability_async.txt", "rb");
		if (fp) {
			char read_buf[256] = { 0 };
			size_t n = fread(read_buf, 1, data_size, fp);
			fclose(fp);
			if (n != data_size ||
			    memcmp(read_buf, data, data_size) != 0) {
				success = false;
			}
		} else {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);
	remove("test_durability_async.txt");

	if (success)
		printf("✓ fun_write_with_async_durability passed\n");
	return success;
}

static bool
test_write_with_sync_durability(void)
{
	remove("test_durability_sync.txt");

	const char *data = "sync test data!";
	size_t data_size = strlen(data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	memcpy(mem_result.value, data, data_size);

	Write params = { .file_path = "test_durability_sync.txt",
			.input = mem_result.value,
			.bytes_to_write = data_size,
			.offset = 0,
			.mode = FILE_MODE_AUTO,
			.durability_mode = FILE_DURABILITY_SYNC,
			.adaptive = NULL };

	AsyncResult result = fun_write_memory_to_file(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_COMPLETED) &&
		(fun_error_is_ok(result.error));

	if (success) {
		FILE *fp = fopen("test_durability_sync.txt", "rb");
		if (fp) {
			char read_buf[256] = { 0 };
			size_t n = fread(read_buf, 1, data_size, fp);
			fclose(fp);
			if (n != data_size ||
			    memcmp(read_buf, data, data_size) != 0) {
				success = false;
			}
		} else {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);
	remove("test_durability_sync.txt");

	if (success)
		printf("✓ fun_write_with_sync_durability passed\n");
	return success;
}

static bool
test_write_with_full_durability(void)
{
	remove("test_durability_full.txt");

	const char *data = "full durability!";
	size_t data_size = strlen(data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	memcpy(mem_result.value, data, data_size);

	Write params = { .file_path = "test_durability_full.txt",
			.input = mem_result.value,
			.bytes_to_write = data_size,
			.offset = 0,
			.mode = FILE_MODE_AUTO,
			.durability_mode = FILE_DURABILITY_FULL,
			.adaptive = NULL };

	AsyncResult result = fun_write_memory_to_file(params);
	fun_async_await(&result, -1);

	bool success = (result.status == ASYNC_COMPLETED) &&
		(fun_error_is_ok(result.error));

	if (success) {
		FILE *fp = fopen("test_durability_full.txt", "rb");
		if (fp) {
			char read_buf[256] = { 0 };
			size_t n = fread(read_buf, 1, data_size, fp);
			fclose(fp);
			if (n != data_size ||
			    memcmp(read_buf, data, data_size) != 0) {
				success = false;
			}
		} else {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);
	remove("test_durability_full.txt");

	if (success)
		printf("✓ fun_write_with_full_durability passed\n");
	return success;
}

static bool
test_append_with_sync_durability(void)
{
	remove("test_append_durability.txt");

	const char *data1 = "Part one. ";
	const char *data2 = "Part two.";
	size_t size1 = strlen(data1);
	size_t size2 = strlen(data2);

	MemoryResult mem1 = fun_memory_allocate(size1);
	if (fun_error_is_error(mem1.error))
		return false;
	memcpy(mem1.value, data1, size1);

	Write write_params = { .file_path = "test_append_durability.txt",
			.input = mem1.value,
			.bytes_to_write = size1,
			.offset = 0,
			.mode = FILE_MODE_AUTO,
			.durability_mode = FILE_DURABILITY_ASYNC,
			.adaptive = NULL };

	AsyncResult wr = fun_write_memory_to_file(write_params);
	fun_async_await(&wr, -1);
	fun_memory_free(&mem1.value);

	if (wr.status != ASYNC_COMPLETED || fun_error_is_error(wr.error)) {
		remove("test_append_durability.txt");
		return false;
	}

	MemoryResult mem2 = fun_memory_allocate(size2);
	if (fun_error_is_error(mem2.error)) {
		remove("test_append_durability.txt");
		return false;
	}
	memcpy(mem2.value, data2, size2);

	Append append_params = { .file_path = "test_append_durability.txt",
			.input = mem2.value,
			.bytes_to_append = size2,
			.mode = FILE_MODE_AUTO,
			.durability_mode = FILE_DURABILITY_SYNC,
			.adaptive = NULL };

	AsyncResult ar = fun_append_memory_to_file(append_params);
	fun_async_await(&ar, -1);

	bool success = (ar.status == ASYNC_COMPLETED) &&
		(fun_error_is_ok(ar.error));

	if (success) {
		FILE *fp = fopen("test_append_durability.txt", "rb");
		if (fp) {
			char buf[256] = { 0 };
			size_t total = size1 + size2;
			size_t n = fread(buf, 1, total, fp);
			fclose(fp);
			char expected[256] = { 0 };
			memcpy(expected, data1, size1);
			memcpy(expected + size1, data2, size2);
			if (n != total || memcmp(buf, expected, total) != 0)
				success = false;
		} else {
			success = false;
		}
	}

	fun_memory_free(&mem2.value);
	remove("test_append_durability.txt");

	if (success)
		printf("✓ fun_append_with_sync_durability passed\n");
	return success;
}

int main(void)
{
	printf("Running durability file tests:\n");

	if (!test_write_with_async_durability()) {
		printf("ASYNC durability write test failed\n");
		return 1;
	}
	if (!test_write_with_sync_durability()) {
		printf("SYNC durability write test failed\n");
		return 1;
	}
	if (!test_write_with_full_durability()) {
		printf("FULL durability write test failed\n");
		return 1;
	}
	if (!test_append_with_sync_durability()) {
		printf("Append with SYNC durability test failed\n");
		return 1;
	}

	printf("All durability file tests passed!\n");
	return 0;
}
