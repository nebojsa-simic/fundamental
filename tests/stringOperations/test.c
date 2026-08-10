#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define MAX_TEST_STRING_LENGTH 256

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

char result[MAX_TEST_STRING_LENGTH];

void test_fun_string_join()
{
	if (fun_string_join("Hello, ", "World!", result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Hello, World!") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_join("", "Empty", result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Empty") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_join("Prefix", "", result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Prefix") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_join");
}

void test_fun_string_length()
{
	StringLength length = fun_string_length("Hello");
	if (!(length == 5)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	length = fun_string_length("");
	if (!(length == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	fun_string_length(NULL);
	if (!(length == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_length");
}

void test_fun_string_compare()
{
	StringDifference difference = fun_string_compare("Hello", "Hello");
	if (!(difference == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	difference = fun_string_compare("Hello", "World");
	if (!(difference < 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	difference = fun_string_compare("", "Hello");
	if (!(difference < 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	difference = fun_string_compare(NULL, "Hello World");
	if (!(difference < 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_compare");
}

void test_fun_string_copy()
{
	if (fun_string_copy("Hello", result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Hello") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_copy("", result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_copy("Hello", result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (fun_string_copy(NULL, result, sizeof(result)).error.code == 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Hello") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_copy");
}

void test_fun_string_trim_in_place()
{
	char untrimmed[] = "  Hello  ";
	fun_string_trim_in_place(untrimmed);
	if (fun_string_is_valid(untrimmed, 10).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(untrimmed, "Hello") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	char trimmed[] = "Hello";
	fun_string_trim_in_place(trimmed);
	if (fun_string_is_valid(trimmed, 6).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(trimmed, "Hello") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	char allWhitespace[] = "   ";
	fun_string_trim_in_place(allWhitespace);
	if (fun_string_is_valid(allWhitespace, 4).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(allWhitespace, "") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	OutputString nullPointer = NULL;
	fun_string_trim_in_place(nullPointer);

	print_test_result("fun_string_trim_in_place");
}

void test_fun_string_reverse_in_place()
{
	char hello[] = "  Hello  ";
	fun_string_reverse_in_place(hello);
	if (fun_string_is_valid(hello, 10).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(hello, "  olleH  ") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	char hellotrimmed[] = "Hello";
	fun_string_reverse_in_place(hellotrimmed);
	if (fun_string_is_valid(hellotrimmed, 6).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(hellotrimmed, "olleH") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	char allWhitespace[] = "   ";
	fun_string_reverse_in_place(allWhitespace);
	if (fun_string_is_valid(allWhitespace, 4).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(allWhitespace, "   ") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	OutputString nullPointer = NULL;
	fun_string_reverse_in_place(nullPointer);

	print_test_result("fun_string_reverse_in_place");
}

void test_fun_string_index_of()
{
	StringPosition position = fun_string_index_of("Hello, World!", "World", 0);
	if (!(position == 7)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	position = fun_string_index_of("Hello, World!", "o", 0);
	if (!(position == 4)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	position = fun_string_index_of("Hello, World!", "o", 5);
	if (!(position == 8)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	position = fun_string_index_of("Hello, World!", "xyz", 0);
	if (!(position == -1)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	position = fun_string_index_of("Hello, World!", "World", 9);
	if (!(position == -1)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	position = fun_string_index_of(NULL, "World", 0);
	if (!(position == -1)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_index_of");
}

int main()
{
	fun_console_write_line("Running stringOperations module tests:");
	test_fun_string_compare();
	test_fun_string_index_of();
	test_fun_string_length();
	test_fun_string_trim_in_place();
	test_fun_string_reverse_in_place();
	test_fun_string_join();
	test_fun_string_copy();
	fun_console_write_line("All tests passed!");
	return 0;
}
