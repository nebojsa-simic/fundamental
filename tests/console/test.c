#include <stdio.h>
#include <assert.h>
#include "fundamental/console/console.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

static void test_console_write_line_empty(void)
{
	ErrorResult result = fun_console_write_line("");
	assert(result.code == ERROR_CODE_NO_ERROR);
	// Visual verification: should output only a newline
	print_test_result("test_console_write_line_empty");
}

static void test_console_flush_empty(void)
{
	ErrorResult result = fun_console_flush();
	assert(result.code == ERROR_CODE_NO_ERROR);
	// Should succeed with no output when buffer is empty
	print_test_result("test_console_flush_empty");
}

static void test_console_write_line_basic(void)
{
	ErrorResult result = fun_console_write_line("Hello, World!");
	assert(result.code == ERROR_CODE_NO_ERROR);
	print_test_result("test_console_write_line_basic");
}

static void test_console_write_line_adds_newline(void)
{
	// This test verifies newline is added by checking output visually
	ErrorResult result = fun_console_write_line("Line with newline");
	assert(result.code == ERROR_CODE_NO_ERROR);
	print_test_result("test_console_write_line_adds_newline");
}

static void test_console_error_line_separate_stream(void)
{
	ErrorResult result = fun_console_error_line("Error message to stderr");
	assert(result.code == ERROR_CODE_NO_ERROR);
	print_test_result("test_console_error_line_separate_stream");
}

static void test_console_write_without_newline(void)
{
	ErrorResult result = fun_console_write("No newline yet");
	assert(result.code == ERROR_CODE_NO_ERROR);
	// Flush to see output
	result = fun_console_flush();
	assert(result.code == ERROR_CODE_NO_ERROR);
	print_test_result("test_console_write_without_newline");
}

static void test_console_write_incremental_build(void)
{
	ErrorResult result;

	result = fun_console_write("Incremental: ");
	assert(result.code == ERROR_CODE_NO_ERROR);

	result = fun_console_write("part1 ");
	assert(result.code == ERROR_CODE_NO_ERROR);

	result = fun_console_write("part2 ");
	assert(result.code == ERROR_CODE_NO_ERROR);

	result = fun_console_write_line("done!");
	assert(result.code == ERROR_CODE_NO_ERROR);

	print_test_result("test_console_write_incremental_build");
}

static void test_console_flush_explicit(void)
{
	ErrorResult result = fun_console_write("Before flush");
	assert(result.code == ERROR_CODE_NO_ERROR);

	result = fun_console_flush();
	assert(result.code == ERROR_CODE_NO_ERROR);

	print_test_result("test_console_flush_explicit");
}

static void test_console_write_long_line(void)
{
	// Create a string longer than 512-byte buffer
	char long_string[600];
	for (int i = 0; i < 599; i++) {
		long_string[i] = 'A';
	}
	long_string[599] = '\0';

	ErrorResult result = fun_console_write_line(long_string);
	assert(result.code == ERROR_CODE_NO_ERROR);
	print_test_result("test_console_write_long_line");
}

static void test_console_write_null_parameter(void)
{
	ErrorResult result = fun_console_write_line(NULL);
	assert(result.code == ERROR_CODE_NULL_POINTER);

	result = fun_console_write(NULL);
	assert(result.code == ERROR_CODE_NULL_POINTER);

	result = fun_console_error_line(NULL);
	assert(result.code == ERROR_CODE_NULL_POINTER);

	print_test_result("test_console_write_null_parameter");
}

int main(void)
{
	printf("Running console module tests:\n");

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

	printf("All console module tests passed.\n");
	return 0;
}
