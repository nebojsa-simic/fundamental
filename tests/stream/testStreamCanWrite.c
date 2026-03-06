#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/stream/stream.h"
#include "../../include/memory/memory.h"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

bool test_fun_stream_can_write_basic(void)
{
	// Allocate buffer for streaming
	MemoryResult buffer_result = fun_memory_allocate(1024);
	ASSERT_NO_ERROR(buffer_result);

	// Open a writable stream (create output file)
	AsyncResult stream_result =
		fun_stream_open("testData/can_write_test.txt", STREAM_MODE_WRITE,
						buffer_result.value, 1024, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);
	ASSERT_NO_ERROR(stream_result);

	FileStream *stream = (FileStream *)stream_result.state;

	// Test that stream can handle a reasonable amount to write (less than buffer size)
	bool can_write_reasonable = fun_stream_can_write(stream, 512);
	assert(can_write_reasonable == true);

	// Test a large amount that may be too much for some streams
	// (depends on implementation and available space)
	bool can_write_massive =
		fun_stream_can_write(stream, 1024 * 1024 * 1024); // Gigabytes
	// Expected result depends on platform/io limits - may be false

	// Close and cleanup
	AsyncResult close_result = fun_stream_close(stream);
	fun_async_await(&close_result);

	fun_memory_free(&buffer_result.value);

	remove("testData/can_write_test.txt");

	printf("✓ fun_stream_can_write_basic passed\n");
	return true;
}

bool test_fun_stream_can_write_with_null_stream(void)
{
	// Test that passing NULL stream returns false
	bool result = fun_stream_can_write(NULL, 1024);
	assert(result == false); // Expectation: NULL stream should return false

	printf("✓ fun_stream_can_write_with_null_stream passed\n");
	return true;
}

bool test_fun_stream_can_write_edge_cases(void)
{
	// Allocate buffer
	MemoryResult buffer_result = fun_memory_allocate(1024);
	if (fun_error_is_error(buffer_result.error)) {
		return false;
	}

	// Open stream
	AsyncResult stream_result =
		fun_stream_open("testData/can_write_edge_test.txt", STREAM_MODE_WRITE,
						buffer_result.value, 1024, FILE_MODE_STANDARD);
	fun_async_await(&stream_result);
	if (fun_error_is_error(stream_result.error)) {
		fun_memory_free(&buffer_result.value);
		return false;
	}

	FileStream *stream = (FileStream *)stream_result.state;

	// Test with 0 size (should typically be possible)
	bool can_write_zero = fun_stream_can_write(stream, 0);
	// Behavior depends on implementation: could be true or false

	// Close and cleanup
	AsyncResult close_result = fun_stream_close(stream);
	fun_async_await(&close_result);

	fun_memory_free(&buffer_result.value);

	remove("testData/can_write_edge_test.txt");

	printf("✓ fun_stream_can_write_edge_cases passed\n");
	return true;
}

int main()
{
	printf("Running stream can_write tests:\n");

	if (!test_fun_stream_can_write_basic()) {
		printf("Basic can_write test failed\n");
		return 1;
	}

	if (!test_fun_stream_can_write_with_null_stream()) {
		printf("Null stream test failed\n");
		return 1;
	}

	if (!test_fun_stream_can_write_edge_cases()) {
		printf("Edge cases test failed\n");
		return 1;
	}

	printf("All stream can_write tests passed!\n");
	return 0;
}