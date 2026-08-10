#include <assert.h>

#include "fundamental/stream/stream.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/console/console.h"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

bool test_fun_stream_write_basic(void)
{
	// Allocate buffer for streaming
	MemoryResult buffer_result = fun_memory_allocate(1024);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	// Create an output test file
	AsyncResult stream_result =
		fun_stream_open("testData/test_output.txt", STREAM_MODE_WRITE,
						buffer_result.value, 1024, FILE_MODE_AUTO);
	fun_async_await(&stream_result, -1);
	if (fun_error_is_error(stream_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;

	// Prepare data to write
	char test_data[] = "Hello, Stream!";
	size_t data_size = fun_string_length(test_data);

	// Write data to stream
	AsyncResult write_result = fun_stream_write(stream, test_data, data_size);

	// Wait for write to complete
	fun_async_await(&write_result, -1);

	if (fun_error_is_error(write_result.error)) {
		fun_stream_close(stream);
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Close stream
	AsyncResult close_result = fun_stream_close(stream);
	fun_async_await(&close_result, -1);
	if (fun_error_is_error(close_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Clean up
	fun_memory_free(&buffer_result.value);

	// Verify file was actually written correctly by checking if we can read it back
	{
		char read_buffer[512] = { 0 };
		Read rp = { .file_path = "testData/test_output.txt",
				    .output = read_buffer,
				    .bytes_to_read = data_size,
				    .offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(rp);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
			fun_memory_compare(read_buffer, test_data, data_size).value != 0) {
			return false;
		}
	}

	fun_console_write_line("✓ fun_stream_write_basic passed");
	return true;
}

bool test_fun_stream_write_large_data(void)
{
	// Allocate large buffer
	MemoryResult buffer_result = fun_memory_allocate(4096);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	// Open stream for writing
	AsyncResult stream_result =
		fun_stream_open("testData/large_output.txt", STREAM_MODE_WRITE,
						buffer_result.value, 4096, FILE_MODE_AUTO);
	fun_async_await(&stream_result, -1);
	if (fun_error_is_error(stream_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;

	// Prepare large amount of data
	unsigned char large_data[2048];
	for (size_t i = 0; i < 2048; i++) {
		large_data[i] = i % 256;
	}

	// Write large data to stream
	AsyncResult write_result = fun_stream_write(stream, large_data, 2048);
	fun_async_await(&write_result, -1);
	if (fun_error_is_error(write_result.error)) {
		fun_stream_close(stream);
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Close stream
	AsyncResult close_result = fun_stream_close(stream);
	fun_async_await(&close_result, -1);
	if (fun_error_is_error(close_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Clean up
	fun_memory_free(&buffer_result.value);

	// Verification by reading back
	{
		unsigned char read_buffer[2048] = { 0 };
		Read rp2 = { .file_path = "testData/large_output.txt",
					 .output = read_buffer,
					 .bytes_to_read = 2048,
					 .offset = 0 };
		AsyncResult rr = fun_read_file_in_memory(rp2);
		fun_async_await(&rr, -1);
		if (rr.status != ASYNC_COMPLETED ||
			fun_memory_compare(read_buffer, large_data, 2048).value != 0) {
			return false;
		}
	}

	fun_console_write_line("✓ fun_stream_write_large_data passed");
	return true;
}

bool test_fun_stream_write_null_parameters(void)
{
	// Test NULL stream parameter
	AsyncResult write_result = fun_stream_write(NULL, "test", 4);
	if (!fun_error_is_error(write_result.error)) {
		return false; // Should fail with NULL stream
	}
	assert(write_result.status == ASYNC_ERROR);

	// Test NULL data parameter
	MemoryResult buffer_result = fun_memory_allocate(1024);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result =
		fun_stream_open("testData/null_params_test.txt", STREAM_MODE_WRITE,
						buffer_result.value, 1024, FILE_MODE_AUTO);
	fun_async_await(&stream_result, -1);
	if (fun_error_is_error(stream_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;

	write_result = fun_stream_write(stream, NULL, 10); // NULL data ptr
	if (!fun_error_is_error(write_result.error)) {
		fun_stream_close(stream);
		fun_memory_free(&buffer_result.value);
		return false; // Should fail with NULL data
	}
	assert(write_result.status == ASYNC_ERROR);

	// Close stream
	AsyncResult close_result = fun_stream_close(stream);
	fun_async_await(&close_result, -1);
	if (fun_error_is_error(close_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	fun_memory_free(&buffer_result.value);

	fun_console_write_line("✓ fun_stream_write_null_parameters passed");
	return true;
}
