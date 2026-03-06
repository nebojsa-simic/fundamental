#include "stream/stream.h"
#include "memory/memory.h"
#include "string/string.h"
#include <stdio.h>
#include <assert.h>

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define RUN_TEST(test_func)     \
	printf("%s\n", #test_func); \
	assert(test_func());        \
	printf("%s %s\n", GREEN_CHECK, #test_func);

// Test declarations
bool test_stream_create_valid_file(void);
bool test_stream_create_invalid_file(void);
bool test_stream_read_small_file(void);
bool test_stream_read_empty_file(void);
bool test_stream_read_large_file_multiple_chunks(void);
bool test_stream_read_exact_buffer_size(void);
bool test_stream_read_null_parameters(void);
bool test_stream_read_after_end_of_stream(void);
bool test_stream_position_tracking(void);
bool test_stream_memory_cleanup(void);
bool test_stream_async_behavior(void);

int main(void)
{
	printf("Running stream module tests:\n");

	// Lifecycle tests
	RUN_TEST(test_stream_create_valid_file);
	RUN_TEST(test_stream_create_invalid_file);

	// Core functionality tests
	RUN_TEST(test_stream_read_small_file);
	RUN_TEST(test_stream_read_empty_file);
	RUN_TEST(test_stream_read_large_file_multiple_chunks);
	RUN_TEST(test_stream_read_exact_buffer_size);

	// Error condition tests
	RUN_TEST(test_stream_read_null_parameters);
	RUN_TEST(test_stream_read_after_end_of_stream);

	// Advanced tests
	RUN_TEST(test_stream_position_tracking);
	RUN_TEST(test_stream_memory_cleanup);
	RUN_TEST(test_stream_async_behavior);

	printf("All tests passed!\n");
}
