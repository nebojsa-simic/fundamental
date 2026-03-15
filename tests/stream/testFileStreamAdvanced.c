#include "stream/stream.h"
#include "memory/memory.h"
#include <stdio.h>

bool test_stream_position_tracking(void)
{
	MemoryResult buffer_result = fun_memory_allocate(256);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result =
		fun_stream_create_file_read("testData/medium.txt", // 1KB file
									buffer_result.value, 256, FILE_MODE_AUTO);
	fun_async_await(&stream_result, -1);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	uint64_t bytes_read;

	// Initial position should be 0
	if (fun_stream_current_position(stream) != 0) {
		fun_stream_destroy(stream);
		fun_memory_free(&buffer_result.value);
		return false;
	}

	// Read first chunk
	AsyncResult read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result, -1);

	if (read_result.status != ASYNC_COMPLETED ||
		fun_stream_current_position(stream) != bytes_read) {
		fun_stream_destroy(stream);
		fun_memory_free(&buffer_result.value);
		return false;
	}

	uint64_t first_position = fun_stream_current_position(stream);

	// Read second chunk
	read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result, -1);

	bool success =
		(read_result.status == ASYNC_COMPLETED) &&
		(fun_stream_current_position(stream) == first_position + bytes_read);

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);
	return success;
}

bool test_stream_async_behavior(void)
{
	// Test that async operations behave correctly
	MemoryResult buffer_result = fun_memory_allocate(512);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	AsyncResult stream_result = fun_stream_create_file_read(
		"testData/medium.txt", buffer_result.value, 512, FILE_MODE_AUTO);
	fun_async_await(&stream_result, -1);

	if (stream_result.status != ASYNC_COMPLETED) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;
	uint64_t bytes_read;

	AsyncResult read_result = fun_stream_read(stream, &bytes_read);
	fun_async_await(&read_result, -1);

	// fun_stream_read is synchronous, so it returns ASYNC_COMPLETED
	// The async mechanism is used for consistency with the API design
	bool success = (read_result.status == ASYNC_COMPLETED) && (bytes_read > 0);

	fun_stream_destroy(stream);
	fun_memory_free(&buffer_result.value);
	return success;
}
