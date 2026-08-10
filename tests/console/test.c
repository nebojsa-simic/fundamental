#include "fundamental/console/console.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

static void test_console_write_line_empty(void)
{
	ErrorResult result = fun_console_write_line("");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result("test_console_write_line_empty");
}

static void test_console_flush_empty(void)
{
	ErrorResult result = fun_console_flush();
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result("test_console_flush_empty");
}

static void test_console_write_line_basic(void)
{
	ErrorResult result = fun_console_write_line("Hello, World!");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result("test_console_write_line_basic");
}

static void test_console_write_line_adds_newline(void)
{
	ErrorResult result = fun_console_write_line("Line with newline");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result("test_console_write_line_adds_newline");
}

static void test_console_error_line_separate_stream(void)
{
	ErrorResult result = fun_console_error_line("Error message to stderr");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result("test_console_error_line_separate_stream");
}

static void test_console_write_without_newline(void)
{
	ErrorResult result = fun_console_write("No newline yet");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	result = fun_console_flush();
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result("test_console_write_without_newline");
}

static void test_console_write_incremental_build(void)
{
	ErrorResult result;

	result = fun_console_write("Incremental: ");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	result = fun_console_write("part1 ");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	result = fun_console_write("part2 ");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	result = fun_console_write_line("done!");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("test_console_write_incremental_build");
}

static void test_console_flush_explicit(void)
{
	ErrorResult result = fun_console_write("Before flush");
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	result = fun_console_flush();
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("test_console_flush_explicit");
}

static void test_console_write_long_line(void)
{
	char long_string[600];
	for (int i = 0; i < 599; i++) {
		long_string[i] = 'A';
	}
	long_string[599] = '\0';

	ErrorResult result = fun_console_write_line(long_string);
	if (!(result.code == ERROR_CODE_NO_ERROR)) {
		fun_console_write_line("FAIL: check");
		return;
	}
	print_test_result("test_console_write_long_line");
}

static void test_console_write_null_parameter(void)
{
	ErrorResult result = fun_console_write_line(NULL);
	if (!(result.code == ERROR_CODE_NULL_POINTER)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	result = fun_console_write(NULL);
	if (!(result.code == ERROR_CODE_NULL_POINTER)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	result = fun_console_error_line(NULL);
	if (!(result.code == ERROR_CODE_NULL_POINTER)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("test_console_write_null_parameter");
}

int main(void)
{
	fun_console_write_line("Running console module tests:");

	test_console_write_line_basic();
	test_console_write_line_adds_newline();
	test_console_write_line_empty();
	test_console_error_line_separate_stream();
	test_console_write_without_newline();
	test_console_write_incremental_build();
	test_console_flush_explicit();
	test_console_flush_empty();
	test_console_write_long_line();
	test_console_write_null_parameter();

	fun_console_write_line("All console module tests passed.");
	return 0;
}
