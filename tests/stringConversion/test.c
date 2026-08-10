#include "fundamental/console/console.h"
#include "fundamental/error/error.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define MAX_TEST_STRING_LENGTH 32

char result[MAX_TEST_STRING_LENGTH];

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

void test_fun_string_from_int()
{
	if (fun_string_from_int(12345, 10, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "12345") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_int(-987654321, 10, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "-987654321") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_int(0, 10, result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "0") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_int(255, 16, result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "ff") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_int(1234567890123456789LL, 10, result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "1234567890123456789") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_from_int");
}

void test_fun_string_from_double()
{
	if (fun_string_from_double(123.456, 0, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "123") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_double(123.456, 3, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "123.456") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_double(-78.9, 1, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "-78.9") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_double(0.0, 2, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "0.00") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_double(1234567890.123456, 6, result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "1234567890.123456") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_double(0.000001, 6, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "0.000001") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_from_double(1.23456789, 2, result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "1.23") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_from_double");
}

void test_fun_string_from_pointer()
{
	if (fun_string_from_pointer(NULL, result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "0x0000000000000000") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	void *ptr = (void *)0x12345678;
	if (fun_string_from_pointer(ptr, result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "0x0000000012345678") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	ptr = (void *)0xffffffffffffffff;
	if (fun_string_from_pointer(ptr, result, sizeof(result)).error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "0xffffffffffffffff") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_from_pointer");
}

int main()
{
	fun_console_write_line("Running stringConversion module tests:");
	test_fun_string_from_int();
	test_fun_string_from_double();
	test_fun_string_from_pointer();
	fun_console_write_line("All tests passed!");
	return 0;
}
