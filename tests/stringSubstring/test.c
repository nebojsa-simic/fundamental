#include "fundamental/console/console.h"
#include "fundamental/string/string.h"

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		fun_console_write(GREEN_CHECK);
		fun_console_write(" ");
		fun_console_write_line(test_name);
		tests_passed++;
	} else {
		fun_console_write(RED_CROSS);
		fun_console_write(" ");
		fun_console_write_line(test_name);
		tests_failed++;
	}
}

static void test_substring_basic(void)
{
	char output[32];
	voidResult result =
		fun_string_substring("Hello World", 6, 5, output, sizeof(output));
	print_test_result("fun_string_substring - basic extraction",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "World") == 0);
}

static void test_substring_from_start(void)
{
	char output[32];
	voidResult result =
		fun_string_substring("Hello World", 0, 5, output, sizeof(output));
	print_test_result("fun_string_substring - from start",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "Hello") == 0);
}

static void test_substring_full_string(void)
{
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 0, 5, output, sizeof(output));
	print_test_result("fun_string_substring - full string",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "Hello") == 0);
}

static void test_substring_middle(void)
{
	char output[32];
	voidResult result =
		fun_string_substring("Programming", 3, 4, output, sizeof(output));
	print_test_result("fun_string_substring - middle extraction",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "gram") == 0);
}

static void test_substring_null_source(void)
{
	char output[32];
	voidResult result =
		fun_string_substring(NULL, 0, 5, output, sizeof(output));
	print_test_result("fun_string_substring - NULL source",
					  result.error.code == ERROR_CODE_NULL_POINTER);
}

static void test_substring_null_output(void)
{
	voidResult result = fun_string_substring("Hello", 0, 5, NULL, 32);
	print_test_result("fun_string_substring - NULL output",
					  result.error.code == ERROR_CODE_NULL_POINTER);
}

static void test_substring_start_out_of_bounds(void)
{
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 10, 5, output, sizeof(output));
	print_test_result("fun_string_substring - start out of bounds",
					  result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);
}

static void test_substring_length_exceeds(void)
{
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 3, 10, output, sizeof(output));
	print_test_result("fun_string_substring - length exceeds source",
					  result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);
}

static void test_substring_buffer_too_small(void)
{
	char output[3];
	voidResult result =
		fun_string_substring("Hello", 0, 5, output, sizeof(output));
	print_test_result("fun_string_substring - buffer too small",
					  result.error.code == ERROR_CODE_BUFFER_TOO_SMALL);
}

static void test_substring_zero_length(void)
{
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 2, 0, output, sizeof(output));
	print_test_result("fun_string_substring - zero length",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  output[0] == '\0');
}

static void test_slice_basic(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 0, 5, output, sizeof(output));
	print_test_result("fun_string_slice - basic extraction",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "Hello") == 0);
}

static void test_slice_to_end(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 6, 11, output, sizeof(output));
	print_test_result("fun_string_slice - to end",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "World") == 0);
}

static void test_slice_negative_start(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", -5, 11, output, sizeof(output));
	print_test_result("fun_string_slice - negative start",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "World") == 0);
}

static void test_slice_negative_end(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 0, -6, output, sizeof(output));
	print_test_result("fun_string_slice - negative end",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "Hello") == 0);
}

static void test_slice_both_negative(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", -5, -1, output, sizeof(output));
	print_test_result("fun_string_slice - both negative",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "Worl") == 0);
}

static void test_slice_start_ge_end(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 5, 3, output, sizeof(output));
	print_test_result("fun_string_slice - start >= end",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  output[0] == '\0');
}

static void test_slice_null_source(void)
{
	char output[32];
	voidResult result = fun_string_slice(NULL, 0, 5, output, sizeof(output));
	print_test_result("fun_string_slice - NULL source",
					  result.error.code == ERROR_CODE_NULL_POINTER);
}

static void test_slice_null_output(void)
{
	voidResult result = fun_string_slice("Hello", 0, 5, NULL, 32);
	print_test_result("fun_string_slice - NULL output",
					  result.error.code == ERROR_CODE_NULL_POINTER);
}

static void test_slice_start_out_of_bounds(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello", -20, 3, output, sizeof(output));
	print_test_result("fun_string_slice - start out of bounds",
					  result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);
}

static void test_slice_end_out_of_bounds(void)
{
	char output[32];
	voidResult result =
		fun_string_slice("Hello", 0, 20, output, sizeof(output));
	print_test_result("fun_string_slice - end out of bounds",
					  result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);
}

static void test_slice_buffer_too_small(void)
{
	char output[3];
	voidResult result = fun_string_slice("Hello", 0, 5, output, sizeof(output));
	print_test_result("fun_string_slice - buffer too small",
					  result.error.code == ERROR_CODE_BUFFER_TOO_SMALL);
}

static void test_slice_full_string(void)
{
	char output[32];
	voidResult result = fun_string_slice("Hello", 0, 5, output, sizeof(output));
	print_test_result("fun_string_slice - full string",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  fun_string_compare(output, "Hello") == 0);
}

static void test_slice_empty_result(void)
{
	char output[32];
	voidResult result = fun_string_slice("Hello", 3, 3, output, sizeof(output));
	print_test_result("fun_string_slice - same start and end",
					  result.error.code == ERROR_CODE_NO_ERROR &&
						  output[0] == '\0');
}

int main(void)
{
	fun_console_write_line("Running stringSubstring module tests:");

	test_substring_basic();
	test_substring_from_start();
	test_substring_full_string();
	test_substring_middle();
	test_substring_null_source();
	test_substring_null_output();
	test_substring_start_out_of_bounds();
	test_substring_length_exceeds();
	test_substring_buffer_too_small();
	test_substring_zero_length();

	test_slice_basic();
	test_slice_to_end();
	test_slice_negative_start();
	test_slice_negative_end();
	test_slice_both_negative();
	test_slice_start_ge_end();
	test_slice_null_source();
	test_slice_null_output();
	test_slice_start_out_of_bounds();
	test_slice_end_out_of_bounds();
	test_slice_buffer_too_small();
	test_slice_full_string();
	test_slice_empty_result();

	if (tests_failed == 0) {
		fun_console_write_line("All stringSubstring tests passed!");
	} else {
		fun_console_write("Tests passed: ");
		char num_buf[32];
		fun_string_from_int(tests_passed, 10, num_buf, sizeof(num_buf));
		fun_console_write(num_buf);
		fun_console_write(", failed: ");
		fun_string_from_int(tests_failed, 10, num_buf, sizeof(num_buf));
		fun_console_write_line(num_buf);
	}

	return tests_failed > 0 ? 1 : 0;
}
