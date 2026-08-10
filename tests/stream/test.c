#include "fundamental/stream/stream.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include "fundamental/console/console.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define RUN_TEST(test_func)                                         \
	fun_console_write_line(#test_func);                              \
	if (!(test_func())) {                                           \
		fun_console_error_line("FAILED");                           \
		return 1;                                                   \
	}                                                               \
	fun_console_write(GREEN_CHECK);                                 \
	fun_console_write(" ");                                         \
	fun_console_write_line(#test_func);

bool test_stream_create_valid_file(void);
bool test_stream_create_invalid_file(void);

bool test_stream_read_small_file(void);
bool test_stream_read_empty_file(void);
bool test_stream_read_large_file_multiple_chunks(void);
bool test_stream_read_exact_buffer_size(void);
bool test_stream_read_null_parameters(void);
bool test_stream_read_after_end_of_stream(void);

bool test_fun_stream_write_basic(void);
bool test_fun_stream_write_large_data(void);
bool test_fun_stream_write_null_parameters(void);
bool test_fun_stream_can_write_basic(void);
bool test_fun_stream_can_write_with_null_stream(void);
bool test_fun_stream_can_write_edge_cases(void);

bool test_stream_position_tracking(void);
bool test_stream_memory_cleanup(void);
bool test_stream_async_behavior(void);

int main(void)
{
	fun_console_write_line("Running stream module tests:");

	RUN_TEST(test_stream_create_valid_file);
	RUN_TEST(test_stream_create_invalid_file);

	RUN_TEST(test_stream_read_small_file);
	RUN_TEST(test_stream_read_empty_file);
	RUN_TEST(test_stream_read_large_file_multiple_chunks);
	RUN_TEST(test_stream_read_exact_buffer_size);

	RUN_TEST(test_fun_stream_write_basic);
	RUN_TEST(test_fun_stream_write_large_data);
	RUN_TEST(test_fun_stream_write_null_parameters);
	RUN_TEST(test_fun_stream_can_write_basic);
	RUN_TEST(test_fun_stream_can_write_with_null_stream);
	RUN_TEST(test_fun_stream_can_write_edge_cases);

	RUN_TEST(test_stream_read_null_parameters);
	RUN_TEST(test_stream_read_after_end_of_stream);

	RUN_TEST(test_stream_position_tracking);
	RUN_TEST(test_stream_memory_cleanup);
	RUN_TEST(test_stream_async_behavior);

	fun_console_write_line("All tests passed!");
	return 0;
}
