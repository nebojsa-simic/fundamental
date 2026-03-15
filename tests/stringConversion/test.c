#include <assert.h>
#include <stdio.h>
#include "../../include/error/error.h"
#include "../../include/string/string.h"
#include "../../include/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

#define MAX_TEST_STRING_LENGTH 32

char result[MAX_TEST_STRING_LENGTH];

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_fun_string_from_int()
{
	// Test positive integer
	ASSERT_NO_ERROR(fun_string_from_int(12345, 10, result, sizeof(result)));
	assert(fun_string_compare(result, "12345") == 0);

	// Test negative integer
	ASSERT_NO_ERROR(
		fun_string_from_int(-987654321, 10, result, sizeof(result)));
	assert(fun_string_compare(result, "-987654321") == 0);

	// Test zero
	ASSERT_NO_ERROR(fun_string_from_int(0, 10, result, sizeof(result)));
	assert(fun_string_compare(result, "0") == 0);

	// Test base 16
	ASSERT_NO_ERROR(fun_string_from_int(255, 16, result, sizeof(result)));
	assert(fun_string_compare(result, "ff") == 0);

	// Test large number
	ASSERT_NO_ERROR(
		fun_string_from_int(1234567890123456789LL, 10, result, sizeof(result)));
	assert(fun_string_compare(result, "1234567890123456789") == 0);

	print_test_result("fun_string_from_int");
}

void test_fun_string_from_double()
{
	// Test positive float with no decimals after point
	ASSERT_NO_ERROR(fun_string_from_double(123.456, 0, result, sizeof(result)));
	assert(fun_string_compare(result, "123") == 0);

	// Test positive float
	ASSERT_NO_ERROR(fun_string_from_double(123.456, 3, result, sizeof(result)));
	assert(fun_string_compare(result, "123.456") == 0);

	// Test negative float
	ASSERT_NO_ERROR(fun_string_from_double(-78.9, 1, result, sizeof(result)));
	assert(fun_string_compare(result, "-78.9") == 0);

	// Test zero
	ASSERT_NO_ERROR(fun_string_from_double(0.0, 2, result, sizeof(result)));
	assert(fun_string_compare(result, "0.00") == 0);

	// Test large number
	ASSERT_NO_ERROR(
		fun_string_from_double(1234567890.123456, 6, result, sizeof(result)));
	assert(fun_string_compare(result, "1234567890.123456") == 0);

	// Test small number
	ASSERT_NO_ERROR(
		fun_string_from_double(0.000001, 6, result, sizeof(result)));
	assert(fun_string_compare(result, "0.000001") == 0);

	// Test rounding
	ASSERT_NO_ERROR(
		fun_string_from_double(1.23456789, 2, result, sizeof(result)));
	assert(fun_string_compare(result, "1.23") == 0);

	print_test_result("fun_string_from_double");
}

void test_fun_string_from_pointer()
{
	// Test NULL pointer
	ASSERT_NO_ERROR(fun_string_from_pointer(NULL, result, sizeof(result)));
	assert(fun_string_compare(result, "0x0000000000000000") == 0);

	// Test non-NULL pointer
	void *ptr = (void *)0x12345678;
	ASSERT_NO_ERROR(fun_string_from_pointer(ptr, result, sizeof(result)));
	assert(fun_string_compare(result, "0x0000000012345678") == 0);

	// Test max pointer value
	ptr = (void *)0xffffffffffffffff; // All bits set to 1
	ASSERT_NO_ERROR(fun_string_from_pointer(ptr, result, sizeof(result)));
	assert(fun_string_compare(result, "0xffffffffffffffff") ==
		   0); // Assuming 64-bit system

	print_test_result("fun_string_from_pointer");
}

int main()
{
	printf("Running stringConversion module tests:\n");
	test_fun_string_from_int();
	test_fun_string_from_double();
	test_fun_string_from_pointer();
	printf("All tests passed!\n");
	return 0;
}
