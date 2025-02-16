#include <assert.h>
#include <stdio.h>
#include "../../src/string/string.h"
#include "../../src/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define MAX_TEST_STRING_LENGTH 256

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(res) assert(res.error.code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

char result[MAX_TEST_STRING_LENGTH];

void test_fun_string_join()
{
	fun_string_join("Hello, ", "World!", result);
	ASSERT_NO_ERROR(fun_string_is_valid(result, MAX_TEST_STRING_LENGTH));
	assert(fun_string_compare(result, "Hello, World!") == 0);

	fun_string_join("", "Empty", result);
	ASSERT_NO_ERROR(fun_string_is_valid(result, MAX_TEST_STRING_LENGTH));
	assert(fun_string_compare(result, "Empty") == 0);

	fun_string_join("Prefix", "", result);
	ASSERT_NO_ERROR(fun_string_is_valid(result, MAX_TEST_STRING_LENGTH));
	assert(fun_string_compare(result, "Prefix") == 0);

	print_test_result("fun_string_join");
}

void test_fun_string_length()
{
	StringLength length = fun_string_length("Hello");
	assert(length == 5);

	length = fun_string_length("");
	assert(length == 0);

	fun_string_length(NULL);
	assert(length == 0);

	print_test_result("fun_string_length");
}

void test_fun_string_compare()
{
	StringDifference difference = fun_string_compare("Hello", "Hello");
	assert(difference == 0);

	difference = fun_string_compare("Hello", "World");
	assert(difference < 0);

	difference = fun_string_compare("", "Hello");
	assert(difference < 0);

	difference = fun_string_compare(NULL, "Hello World");
	assert(difference < 0);

	print_test_result("fun_string_compare");
}

void test_fun_string_copy()
{
	fun_string_copy("Hello", result);
	ASSERT_NO_ERROR(fun_string_is_valid(result, MAX_TEST_STRING_LENGTH));
	assert(fun_string_compare(result, "Hello") == 0);

	fun_string_copy("", result);
	ASSERT_NO_ERROR(fun_string_is_valid(result, MAX_TEST_STRING_LENGTH));
	assert(fun_string_compare(result, "") == 0);

	fun_string_copy("Hello", result);
	fun_string_copy(NULL, result);
	ASSERT_NO_ERROR(fun_string_is_valid(result, MAX_TEST_STRING_LENGTH));
	assert(fun_string_compare(result, "Hello") == 0);

	print_test_result("fun_string_copy");
}

void test_fun_string_trim_in_place()
{
	char untrimmed[] = "  Hello  ";
	fun_string_trim_in_place(untrimmed);
	ASSERT_NO_ERROR(fun_string_is_valid(untrimmed, 10));
	assert(fun_string_compare(untrimmed, "Hello") == 0);

	char trimmed[] = "Hello";
	fun_string_trim_in_place(trimmed);
	ASSERT_NO_ERROR(fun_string_is_valid(trimmed, 6));
	assert(fun_string_compare(trimmed, "Hello") == 0);

	char allWhitespace[] = "   ";
	fun_string_trim_in_place(allWhitespace);
	ASSERT_NO_ERROR(fun_string_is_valid(allWhitespace, 4));
	assert(fun_string_compare(allWhitespace, "") == 0);

	OutputString nullPointer = NULL;
	fun_string_trim_in_place(nullPointer);

	print_test_result("fun_string_trim_in_place");
}

void test_fun_string_reverse_in_place()
{
	char hello[] = "  Hello  ";
	fun_string_reverse_in_place(hello);
	ASSERT_NO_ERROR(fun_string_is_valid(hello, 10));
	assert(fun_string_compare(hello, "  olleH  ") == 0);

	char hellotrimmed[] = "Hello";
	fun_string_reverse_in_place(hellotrimmed);
	ASSERT_NO_ERROR(fun_string_is_valid(hellotrimmed, 6));
	assert(fun_string_compare(hellotrimmed, "olleH") == 0);

	char allWhitespace[] = "   ";
	fun_string_reverse_in_place(allWhitespace);
	ASSERT_NO_ERROR(fun_string_is_valid(allWhitespace, 4));
	assert(fun_string_compare(allWhitespace, "   ") == 0);

	OutputString nullPointer = NULL;
	fun_string_reverse_in_place(nullPointer);

	print_test_result("fun_string_reverse_in_place");
}

void test_fun_string_index_of()
{
	StringPosition position = fun_string_index_of("Hello, World!", "World", 0);
	assert(position == 7);

	position = fun_string_index_of("Hello, World!", "o", 0);
	assert(position == 4);

	position = fun_string_index_of("Hello, World!", "o", 5);
	assert(position == 8);

	position = fun_string_index_of("Hello, World!", "xyz", 0);
	assert(position == -1);

	position = fun_string_index_of("Hello, World!", "World", 9);
	assert(position == -1);

	position = fun_string_index_of(NULL, "World", 0);
	assert(position == -1);

	print_test_result("fun_string_index_of");
}

int main()
{
	printf("Running stringOperations module tests:\n");
	test_fun_string_compare();
	test_fun_string_index_of();
	test_fun_string_length();
	test_fun_string_trim_in_place();
	test_fun_string_reverse_in_place();
	test_fun_string_join();
	test_fun_string_copy();
	printf("All tests passed!\n");
	return 0;
}
