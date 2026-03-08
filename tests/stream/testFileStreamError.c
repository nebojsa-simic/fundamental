#include "stream/stream.h"
#include "memory/memory.h"
#include <stdio.h>

bool test_stream_read_null_parameters(void)
{
	// Test null stream parameter
	uint64_t bytes_read;
	AsyncResult result = fun_stream_read(NULL, &bytes_read);

	if (result.status != ASYNC_ERROR ||
		result.error.code != ERROR_CODE_NULL_POINTER) {
		return false;
	}

	// Test null bytes_read parameter
	MemoryResult buffer_result = fun_memory_allocate(256);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/small.txt", buffer_result.value, 256, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	result = fun_stream_read(stream, NULL);

	bool success = (result.status == ASYNC_ERROR) &&
				   (result.error.code == ERROR_CODE_NULL_POINTER);

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);

	return success;
}

bool test_stream_read_after_end_of_stream(void)
{
	MemoryResult buffer_result = fun_memory_allocate(128);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/small.txt", buffer_result.value, 128, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
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
		return false;
	}

	// Try to read again after EOF
	read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result);

	bool success = (read_result.status == ASYNC_COMPLETED) &&
				   (bytes_read == 0) && (!fun_stream_can_read(stream));

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);
	return success;
}
