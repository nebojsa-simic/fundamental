#include "string/string.h"
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

static void test_slice_basic(void)
{
	const char *test_name = "fun_string_slice - basic extraction";
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "Hello") == 0);

	print_test_result(test_name, passed);
}

static void test_slice_to_end(void)
{
	const char *test_name = "fun_string_slice - to end";
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 6, 11, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "World") == 0);

	print_test_result(test_name, passed);
}

static void test_slice_negative_start(void)
{
	const char *test_name = "fun_string_slice - negative start";
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", -5, 11, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "World") == 0);

	print_test_result(test_name, passed);
}

static void test_slice_negative_end(void)
{
	const char *test_name = "fun_string_slice - negative end";
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 0, -6, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "Hello") == 0);

	print_test_result(test_name, passed);
}

static void test_slice_both_negative(void)
{
	const char *test_name = "fun_string_slice - both negative";
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", -5, -1, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "Worl") == 0);

	print_test_result(test_name, passed);
}

static void test_slice_start_ge_end(void)
{
	const char *test_name = "fun_string_slice - start >= end";
	char output[32];
	voidResult result =
		fun_string_slice("Hello World", 5, 3, output, sizeof(output));

	int passed =
		(result.error.code == ERROR_CODE_NO_ERROR && output[0] == '\0');

	print_test_result(test_name, passed);
}

static void test_slice_null_source(void)
{
	const char *test_name = "fun_string_slice - NULL source";
	char output[32];
	voidResult result = fun_string_slice(NULL, 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NULL_POINTER);

	print_test_result(test_name, passed);
}

static void test_slice_null_output(void)
{
	const char *test_name = "fun_string_slice - NULL output";
	voidResult result = fun_string_slice("Hello", 0, 5, NULL, 32);

	int passed = (result.error.code == ERROR_CODE_NULL_POINTER);

	print_test_result(test_name, passed);
}

static void test_slice_start_out_of_bounds(void)
{
	const char *test_name = "fun_string_slice - start out of bounds";
	char output[32];
	voidResult result =
		fun_string_slice("Hello", -20, 3, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);

	print_test_result(test_name, passed);
}

static void test_slice_end_out_of_bounds(void)
{
	const char *test_name = "fun_string_slice - end out of bounds";
	char output[32];
	voidResult result =
		fun_string_slice("Hello", 0, 20, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_INDEX_OUT_OF_BOUNDS);

	print_test_result(test_name, passed);
}

static void test_slice_buffer_too_small(void)
{
	const char *test_name = "fun_string_slice - buffer too small";
	char output[3];
	voidResult result = fun_string_slice("Hello", 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_BUFFER_TOO_SMALL);

	print_test_result(test_name, passed);
}

static void test_slice_full_string(void)
{
	const char *test_name = "fun_string_slice - full string";
	char output[32];
	voidResult result = fun_string_slice("Hello", 0, 5, output, sizeof(output));

	int passed = (result.error.code == ERROR_CODE_NO_ERROR &&
				  strcmp(output, "Hello") == 0);

	print_test_result(test_name, passed);
}

static void test_slice_empty_result(void)
{
	const char *test_name = "fun_string_slice - same start and end";
	char output[32];
	voidResult result = fun_string_slice("Hello", 3, 3, output, sizeof(output));

	int passed =
		(result.error.code == ERROR_CODE_NO_ERROR && output[0] == '\0');

	print_test_result(test_name, passed);
}

int main(void)
{
	printf("=== Slice Tests ===\n\n");

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

	printf("\n=== Results ===\n");
	printf("Passed: %d\n", tests_passed);
	printf("Failed: %d\n", tests_failed);

	return tests_failed > 0 ? 1 : 0;
}
