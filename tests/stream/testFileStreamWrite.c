#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/stream/stream.h"
#include "../../include/memory/memory.h"

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
						buffer_result.value, 1024, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);
	if (fun_error_is_error(stream_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;

	// Prepare data to write
	char test_data[] = "Hello, Stream!";
	size_t data_size = strlen(test_data);

	// Write data to stream
	AsyncResult write_result = fun_stream_write(stream, test_data, data_size);

	// Wait for write to complete
	fun_async_await(&write_result);

	if (fun_error_is_error(write_result.error)) {
		fun_stream_close(stream);
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Close stream
	AsyncResult close_result = fun_stream_close(stream);
	fun_async_await(&close_result);
	if (fun_error_is_error(close_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Clean up
	fun_memory_free(&buffer_result.value);

	// Verify file was actually written correctly by checking if we can read it back
	FILE *fp = fopen("testData/test_output.txt", "rb");
	if (fp) {
		char read_buffer[512];
		fseek(fp, 0, SEEK_SET);
		int read_bytes = fread(read_buffer, 1, data_size, fp);
		fclose(fp);
		if (read_bytes != data_size ||
			memcmp(read_buffer, test_data, data_size) != 0) {
			return false;
		}
	}

	printf("✓ fun_stream_write_basic passed\n");
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
						buffer_result.value, 4096, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);
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
	fun_async_await(&write_result);
	if (fun_error_is_error(write_result.error)) {
		fun_stream_close(stream);
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Close stream
	AsyncResult close_result = fun_stream_close(stream);
	fun_async_await(&close_result);
	if (fun_error_is_error(close_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Clean up
	fun_memory_free(&buffer_result.value);

	// Verification by reading back
	FILE *fp = fopen("testData/large_output.txt", "rb");
	if (fp) {
		unsigned char read_buffer[2048];
		int read_bytes = fread(read_buffer, 1, 2048, fp);
		fclose(fp);
		if (read_bytes != 2048 || memcmp(read_buffer, large_data, 2048) != 0) {
			return false;
		}
	}

	printf("✓ fun_stream_write_large_data passed\n");
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
						buffer_result.value, 1024, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);
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
	fun_async_await(&close_result);
	if (fun_error_is_error(close_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	fun_memory_free(&buffer_result.value);

	printf("✓ fun_stream_write_null_parameters passed\n");
	return true;
}