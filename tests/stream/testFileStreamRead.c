#include <assert.h>
#include <stdio.h>

#include "stream/stream.h"
#include "memory/memory.h"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

bool test_stream_read_small_file(void)
{
	// Test reading a small file (10 bytes) with larger buffer (64 bytes)
	MemoryResult buffer_result = fun_memory_allocate(64);
	ASSERT_NO_ERROR(buffer_result);

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/small.txt", buffer_result.value, 64, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);
	ASSERT_NO_ERROR(stream_result);

	FileStream *stream = (FileStream *)stream_result.state;
	uint64_t bytes_read;

	// Read the file
	AsyncResult read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result);
	printf("Bytes read: %lu\n", bytes_read);
	printf("error code: %d\n", read_result.error.code);
	printf("error: %s\n", read_result.error.message);
	ASSERT_NO_ERROR(read_result);

	bool success = (read_result.status == ASYNC_COMPLETED) &&
				   (bytes_read == 10) && // File contains exactly 10 bytes
				   (stream->end_of_stream) && // Should be at end
				   (fun_stream_current_position(stream) == 10);
	assert(success);

	// Verify buffer contains expected data
	char *buffer = (char *)stream->buffer;
	success = (buffer[0] == 'T' && buffer[1] == 'e' && buffer[2] == 's' &&
			   buffer[3] == 't');

	ASSERT_NO_ERROR(fun_stream_destroy(stream));
	ASSERT_NO_ERROR(fun_memory_free(&buffer_result.value));

	return success;
}

bool test_stream_read_empty_file(void)
{
	MemoryResult buffer_result = fun_memory_allocate(256);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/empty.txt", buffer_result.value, 256, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	uint64_t bytes_read;

	AsyncResult read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result);

	bool success = (read_result.status == ASYNC_COMPLETED) &&
				   (bytes_read == 0) && (stream->end_of_stream) &&
				   (!fun_stream_can_read(stream));

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);
	return success;
}

bool test_stream_read_large_file_multiple_chunks(void)
{
	// Test reading large file (10KB) with small buffer (512 bytes)
	MemoryResult buffer_result = fun_memory_allocate(512);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/large.txt", buffer_result.value, 512, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	uint64_t total_bytes_read = 0;
	uint64_t read_operations = 0;

	// Read file in chunks
	while (fun_stream_can_read(stream)) {
		uint64_t bytes_read;
		AsyncResult read_result = fun_stream_read(stream, &bytes_read);
		fun_async_await(&read_result);

		if (read_result.status != ASYNC_COMPLETED) {
			fun_stream_destroy(stream);
			fun_memory_free(&buffer_result.value);
			return false;
		}

		total_bytes_read += bytes_read;
		read_operations++;

		// Prevent infinite loop in case of error
		if (read_operations > 50) {
			break;
		}
	}

	bool success =
		(total_bytes_read == 10240) && // 10KB file
		(read_operations == 20) && // 10KB / 512 bytes = 20 operations
		(stream->end_of_stream);

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);
	return success;
}

bool test_stream_read_exact_buffer_size(void)
{
	// Test file that exactly matches buffer size
	MemoryResult buffer_result = fun_memory_allocate(1024);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/exact_buffer.txt", // Exactly 1024 bytes
		buffer_result.value, 1024, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	uint64_t bytes_read;

	// First read should get exactly 1024 bytes
	AsyncResult read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result);

	bool success = (read_result.status == ASYNC_COMPLETED) &&
				   (bytes_read == 1024) && (stream->end_of_stream);

	// Second read should return 0 bytes
	if (success) {
		read_result = fun_stream_read(stream, &bytes_read);
		fun_async_await(&read_result);

		success = (read_result.status == ASYNC_COMPLETED) && (bytes_read == 0);
	}

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);
	return success;
}
