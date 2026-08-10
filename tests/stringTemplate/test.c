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

void test_fun_string_template()
{
	StringTemplateParam params1[] = { { "name", { .stringValue = "Alice" } },
									  { "age", { .intValue = 30 } } };
	if (fun_string_template("Hello, ${name}! You are #{age} years old.",
							params1, 2, result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Hello, Alice! You are 30 years old.") ==
		  0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	void *ptr = (void *)0x12345678;
	StringTemplateParam params2[] = { { "pi", { .doubleValue = 3.14159 } },
									  { "address", { .pointerValue = ptr } } };
	if (fun_string_template(
			"Pi is approximately %{pi} and the pointer is *{address}", params2,
			2, result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(
			  result,
			  "Pi is approximately 3.141 and the pointer is 0x0000000012345678") ==
		  0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	StringTemplateParam params3[] = { { "name", { .stringValue = "Bob" } } };
	if (fun_string_template("Hello, ${name}! Your age is #{age}.", params3, 1,
							result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Hello, Bob! Your age is .") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_template("", params1, 2, result, sizeof(result)).error.code !=
		0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_template("Hello, World!", NULL, 0, result, sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Hello, World!") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_template("Invalid ${syntax", params1, 2, result,
							sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Invalid ${syntax") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	if (fun_string_template(NULL, params1, 2, result, sizeof(result))
			.error.code == 0) {
		fun_console_write_line("FAIL: check");
		return;
	}

	StringTemplateParam params4[] = {
		{ "large", { .intValue = 9223372036854775807LL } }
	};
	if (fun_string_template("Large number: #{large}", params4, 1, result,
							sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "Large number: 9223372036854775807") ==
		  0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	StringTemplateParam params5[] = { { "repeat", { .stringValue = "echo" } } };
	if (fun_string_template("${repeat} ${repeat} ${repeat}", params5, 1, result,
							sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "echo echo echo") == 0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	StringTemplateParam params6[] = {
		{ "str", { .stringValue = "mixed" } },
		{ "num", { .intValue = 42 } },
		{ "dbl", { .doubleValue = 3.14 } },
		{ "ptr", { .pointerValue = (void *)0xABCDEF } }
	};
	if (fun_string_template("${str} #{num} %{dbl} *{ptr}", params6, 4, result,
							sizeof(result))
			.error.code != 0) {
		fun_console_write_line("FAIL: check");
		return;
	}
	if (!(fun_string_compare(result, "mixed 42 3.140 0x0000000000abcdef") ==
		  0)) {
		fun_console_write_line("FAIL: check");
		return;
	}

	print_test_result("fun_string_template");
}

int main()
{
	fun_console_write_line("Running fun_string_template module tests:");
	test_fun_string_template();
	fun_console_write_line("All tests passed!");
	return 0;
}
