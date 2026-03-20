#include "fundamental/string/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, test_name);
		tests_passed++;
	} else {
		printf("%s %s\n", RED_CROSS, test_name);
		tests_failed++;
	}
}

static void test_substring_basic(void)
{
	const char *test_name = "fun_string_substring - basic extraction";
	char output[32];
	voidResult result =
		fun_string_substring("Hello World", 6, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "World") == 0);

	print_test_result(test_name, passed);
}

static void test_substring_from_start(void)
{
	const char *test_name = "fun_string_substring - from start";
	char output[32];
	voidResult result =
		fun_string_substring("Hello World", 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "Hello") == 0);

	print_test_result(test_name, passed);
}

static void test_substring_full_string(void)
{
	const char *test_name = "fun_string_substring - full string";
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "Hello") == 0);

	print_test_result(test_name, passed);
}

static void test_substring_middle(void)
{
	const char *test_name = "fun_string_substring - middle extraction";
	char output[32];
	voidResult result =
		fun_string_substring("Programming", 3, 4, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "gram") == 0);

	print_test_result(test_name, passed);
}

static void test_substring_null_source(void)
{
	const char *test_name = "fun_string_substring - NULL source";
	char output[32];
	voidResult result =
		fun_string_substring(NULL, 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NULL_POINTER);

	print_test_result(test_name, passed);
}

static void test_substring_null_output(void)
{
	const char *test_name = "fun_string_substring - NULL output";
	voidResult result = fun_string_substring("Hello", 0, 5, NULL, 32);

	int passed = (result.error.code == ERROR_CODE_NULL_POINTER);

	print_test_result(test_name, passed);
}

static void test_substring_start_out_of_bounds(void)
{
	const char *test_name = "fun_string_substring - start out of bounds";
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 10, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);

	print_test_result(test_name, passed);
}

static void test_substring_length_exceeds(void)
{
	const char *test_name = "fun_string_substring - length exceeds source";
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 3, 10, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);

	print_test_result(test_name, passed);
}

static void test_substring_buffer_too_small(void)
{
	const char *test_name = "fun_string_substring - buffer too small";
	char output[3];
	voidResult result =
		fun_string_substring("Hello", 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_BUFFER_TOO_SMALL);

	print_test_result(test_name, passed);
}

static void test_substring_zero_length(void)
{
	const char *test_name = "fun_string_substring - zero length";
	char output[32];
	voidResult result =
		fun_string_substring("Hello", 2, 0, output, sizeof(output));

	int passed =
		(result.error.code == ERROR_CODE_NO_ERROR && output[0] == '\0');

	print_test_result(test_name, passed);
}

int main(void)
{
	printf("=== Substring Tests ===\n\n");

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

	printf("\n=== Results ===\n");
	printf("Passed: %d\n", tests_passed);
	printf("Failed: %d\n", tests_failed);

	return tests_failed > 0 ? 1 : 0;
}
