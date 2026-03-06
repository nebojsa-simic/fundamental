#include "stream/stream.h"
#include "memory/memory.h"
#include <stdio.h>

bool test_stream_read_null_parameters(void)
{
	printf("DEBUG: Starting test_stream_read_null_parameters\n");

	// Test null stream parameter
	printf("DEBUG: Testing NULL stream parameter\n");
	uint64_t bytes_read;
	AsyncResult result = fun_stream_read(NULL, &bytes_read);

	printf("DEBUG: result.status = %d, result.error.code = %d\n", result.status,
		   result.error.code);

	if (result.status != ASYNC_ERROR ||
		result.error.code != ERROR_CODE_NULL_POINTER) {
		printf("DEBUG: First test failed (status: %d, error: %d)\n",
			   result.status, result.error.code);
		return false;
	}

	printf("DEBUG: First test passed\n");

	// Test null bytes_read parameter
	printf("DEBUG: Testing NULL bytes_read parameter\n");
	MemoryResult buffer_result = fun_memory_allocate(256);
	if (fun_error_is_error(buffer_result.error)) {
		printf("DEBUG: Failed to allocate buffer\n");
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/small.txt", buffer_result.value, 256, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);

	printf("DEBUG: stream_result.status = %d, stream_result.error.code = %d\n",
		   stream_result.status, stream_result.error.code);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		printf("DEBUG: Failed to create stream (status: %d, error: %d)\n",
			   stream_result.status, stream_result.error.code);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	result = fun_stream_read(stream, NULL);

	printf("DEBUG: result.status = %d, result.error.code = %d\n", result.status,
		   result.error.code);

	bool success = (result.status == ASYNC_ERROR) &&
				   (result.error.code == ERROR_CODE_NULL_POINTER);

	printf("DEBUG: Second test success = %d\n", success);

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);

	printf("DEBUG: Returning %s\n", success ? "true" : "false");
	return success;
}

bool test_stream_read_after_end_of_stream(void)
{
	printf("DEBUG: Starting test_stream_read_after_end_of_stream\n");
	MemoryResult buffer_result = fun_memory_allocate(128);
	if (fun_error_is_error(buffer_result.error)) {
		printf(
			"DEBUG: Failed to allocate buffer in test_stream_read_after_end_of_stream\n");
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/small.txt", buffer_result.value, 128, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		printf(
			"DEBUG: Failed to create stream in test_stream_read_after_end_of_stream (status: %d, error: %d)\n",
			stream_result.status, stream_result.error.code);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	uint64_t bytes_read;

	// Read entire file
	AsyncResult read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result);

	if (read_result.status != ASYNC_COMPLETED || !stream->end_of_stream) {
		fun_stream_destroy(stream);
		fun_memory_free(&buffer_result.value);
		printf(
			"DEBUG: First read failed or not end of stream (status: %d, end_of_stream: %d)\n",
			read_result.status, stream->end_of_stream);
		return false;
	}

	// Try to read again after EOF
	read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result);

	bool success = (read_result.status == ASYNC_COMPLETED) &&
				   (bytes_read == 0) && (!fun_stream_can_read(stream));

	printf("DEBUG: Second read success = %d, bytes_read = %lu, can_read = %d\n",
		   success, bytes_read, fun_stream_can_read(stream));

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);
	return success;
}