#include <assert.h>
#include <stdio.h>
#include "../../src/string/string.h"
#include "../../src/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define MAX_TEST_STRING_LENGTH 256

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

char result[MAX_TEST_STRING_LENGTH];

void test_fun_string_template()
{
	// Test case 1: Basic string template
	StringTemplateParam params1[] = { { "name", { .stringValue = "Alice" } },
									  { "age", { .intValue = 30 } } };
	fun_string_template("Hello, ${name}! You are #{age} years old.", params1, 2,
						result);
	assert(fun_string_compare(result, "Hello, Alice! You are 30 years old.") ==
		   0);

	// Test case 2: Float and pointer values
	void *ptr = (void *)0x12345678;
	StringTemplateParam params2[] = { { "pi", { .doubleValue = 3.14159 } },
									  { "address", { .pointerValue = ptr } } };
	fun_string_template(
		"Pi is approximately %{pi} and the pointer is *{address}", params2, 2,
		result);
	assert(
		fun_string_compare(
			result,
			"Pi is approximately 3.141 and the pointer is 0x0000000012345678") ==
		0);

	// Test case 3: Missing parameter
	StringTemplateParam params3[] = { { "name", { .stringValue = "Bob" } } };
	fun_string_template("Hello, ${name}! Your age is #{age}.", params3, 1,
						result);
	assert(fun_string_compare(result, "Hello, Bob! Your age is .") == 0);

	// Test case 4: Empty template
	fun_string_template("", params1, 2, result);
	assert(fun_string_compare(result, "") == 0);

	// Test case 5: Template with no parameters
	fun_string_template("Hello, World!", NULL, 0, result);
	assert(fun_string_compare(result, "Hello, World!") == 0);

	// Test case 6: Invalid template syntax
	fun_string_template("Invalid ${syntax", params1, 2, result);
	assert(fun_string_compare(result, "Invalid ${syntax") == 0);

	// Test case 7: Don't crash on NULL template
	fun_string_template(NULL, params1, 2, result);

	// Test case 8: Very large number
	StringTemplateParam params4[] = {
		{ "large", { .intValue = 9223372036854775807LL } } // Max int64_t value
	};
	fun_string_template("Large number: #{large}", params4, 1, result);
	assert(fun_string_compare(result, "Large number: 9223372036854775807") ==
		   0);

	// Test case 9: Multiple occurrences of the same parameter
	StringTemplateParam params5[] = { { "repeat", { .stringValue = "echo" } } };
	fun_string_template("${repeat} ${repeat} ${repeat}", params5, 1, result);
	assert(fun_string_compare(result, "echo echo echo") == 0);

	// Test case 10: Mixed parameter types in one template
	StringTemplateParam params6[] = {
		{ "str", { .stringValue = "mixed" } },
		{ "num", { .intValue = 42 } },
		{ "dbl", { .doubleValue = 3.14 } },
		{ "ptr", { .pointerValue = (void *)0xABCDEF } }
	};
	fun_string_template("${str} #{num} %{dbl} *{ptr}", params6, 4, result);
	assert(fun_string_compare(result, "mixed 42 3.140 0x0000000000abcdef") ==
		   0);

	print_test_result("fun_string_template");
}

int main()
{
	printf("Running fun_string_template module tests:\n");
	test_fun_string_template();
	printf("All tests passed!\n");
	return 0;
}
