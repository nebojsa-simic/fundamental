#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"
#include "fundamental/console/console.h"

static bool test_write_with_async_durability(void)
{
	const char *data = "async test data";
	size_t data_size = fun_string_length(data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	fun_memory_copy((Memory)data, mem_result.value, data_size);

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
		char read_buf[256] = { 0 };
		Read readParams = { .file_path = "test_durability_async.txt",
							.output = read_buf,
							.bytes_to_read = data_size,
							.offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(readParams);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
					    fun_memory_compare(read_buf, data, data_size).value != 0) {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);

	if (success)
		fun_console_write_line("✓ fun_write_with_async_durability passed");
	return success;
}

static bool test_write_with_sync_durability(void)
{
	const char *data = "sync test data!";
	size_t data_size = fun_string_length(data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	fun_memory_copy((Memory)data, mem_result.value, data_size);

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
		char read_buf[256] = { 0 };
		Read readParams = { .file_path = "test_durability_sync.txt",
							.output = read_buf,
							.bytes_to_read = data_size,
							.offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(readParams);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
					    fun_memory_compare(read_buf, data, data_size).value != 0) {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);

	if (success)
		fun_console_write_line("✓ fun_write_with_sync_durability passed");
	return success;
}

static bool test_write_with_full_durability(void)
{
	const char *data = "full durability!";
	size_t data_size = fun_string_length(data);

	MemoryResult mem_result = fun_memory_allocate(data_size);
	if (fun_error_is_error(mem_result.error)) {
		return false;
	}
	fun_memory_copy((Memory)data, mem_result.value, data_size);

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
		char read_buf[256] = { 0 };
		Read readParams = { .file_path = "test_durability_full.txt",
							.output = read_buf,
							.bytes_to_read = data_size,
							.offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(readParams);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
					    fun_memory_compare(read_buf, data, data_size).value != 0) {
			success = false;
		}
	}

	fun_memory_free(&mem_result.value);

	if (success)
		fun_console_write_line("✓ fun_write_with_full_durability passed");
	return success;
}

static bool test_append_with_sync_durability(void)
{
	const char *data1 = "Part one. ";
	const char *data2 = "Part two.";
	size_t size1 = fun_string_length(data1);
	size_t size2 = fun_string_length(data2);

	MemoryResult mem1 = fun_memory_allocate(size1);
	if (fun_error_is_error(mem1.error))
		return false;
	fun_memory_copy((Memory)data1, mem1.value, size1);

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
		return false;
	}

	MemoryResult mem2 = fun_memory_allocate(size2);
	if (fun_error_is_error(mem2.error)) {
		return false;
	}
	fun_memory_copy((Memory)data2, mem2.value, size2);

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
		char buf[256] = { 0 };
		size_t total = size1 + size2;
		Read readParams = { .file_path = "test_append_durability.txt",
							.output = buf,
							.bytes_to_read = total,
							.offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(readParams);
		fun_async_await(&rr, -1);
		char expected[256] = { 0 };
		fun_memory_copy((Memory)data1, expected, size1);
		fun_memory_copy((Memory)data2, expected + size1, size2);
		if (rr.status != ASYNC_COMPLETED ||
					    fun_memory_compare(buf, expected, total).value != 0)
			success = false;
	}

	fun_memory_free(&mem2.value);

	if (success)
		fun_console_write_line("✓ fun_append_with_sync_durability passed");
	return success;
}

int main(void)
{
	fun_console_write_line("Running durability file tests:");

	if (!test_write_with_async_durability()) {
		fun_console_write_line("ASYNC durability write test failed");
		return 1;
	}
	if (!test_write_with_sync_durability()) {
		fun_console_write_line("SYNC durability write test failed");
		return 1;
	}
	if (!test_write_with_full_durability()) {
		fun_console_write_line("FULL durability write test failed");
		return 1;
	}
	if (!test_append_with_sync_durability()) {
		fun_console_write_line("Append with SYNC durability test failed");
		return 1;
	}

	fun_console_write_line("All durability file tests passed!");
	return 0;
}
