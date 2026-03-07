#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../../include/array/array.h"

// Helper macros
#define ASSERT_NO_ERROR(result) assert((result).error.code == 0)
#define ASSERT_ERROR(result) assert((result).error.code != 0)

// Test: Array creation - positive capacity
bool test_fun_array_int_create_positive_initial_capacity()
{
	printf("Running fun_array_int_create positive capacity test: ");

	IntArrayResult result = fun_array_int_create(10);

	if (fun_error_is_error(result.error)) {
		printf("FAILED - unexpected creation error: %d\n", result.error.code);
		return false;
	}

	// Verify array count initially 0
	if (result.value.array.count != 0) {
		printf("FAILED - count should be 0 but is %zu\n",
			   result.value.array.count);
		ErrorResult cleanup = fun_array_int_destroy(&result.value);
		(void)cleanup;
		return false;
	}

	// Verify capacity is allocated
	if (result.value.array.capacity < 10) {
		printf("FAILED - capacity should be >=10 but is %zu\n",
			   result.value.array.capacity);
		ErrorResult cleanup = fun_array_int_destroy(&result.value);
		(void)cleanup;
		return false;
	}

	ErrorResult cleanup = fun_array_int_destroy(&result.value);
	if (fun_error_is_ok(cleanup)) {
		printf("PASSED\n");
		return true;
	} else {
		printf("ERROR - cleanup failed: %d\n", cleanup.code);
		return false;
	}
}

// Test: Add elements - non-full array
bool test_fun_array_int_push_non_full_array()
{
	printf("Running fun_array_int_push on non-full array test: ");

	IntArrayResult create_result = fun_array_int_create(5);
	if (fun_error_is_error(create_result.error)) {
		printf("FAILED - couldn't create array: %d\n",
			   create_result.error.code);
		return false;
	}

	IntArray array = create_result.value;
	ErrorResult push_result = fun_array_int_push(&array, 42);

	if (fun_error_is_error(push_result)) {
		printf("FAILED - couldn't push value: %d\n", push_result.code);
		ErrorResult cleanup = fun_array_int_destroy(&array);
		(void)cleanup;
		return false;
	}

	// Verify count increased
	if (array.array.count != 1) {
		printf("FAILED - count should be 1 but is %zu\n", array.array.count);
		ErrorResult cleanup = fun_array_int_destroy(&array);
		(void)cleanup;
		return false;
	}

	// Verify element inserted correctly
	int retrieved = fun_array_int_get(&array, 0);
	if (retrieved != 42) {
		printf("FAILED - expected 42, got %d\n", retrieved);
		ErrorResult cleanup = fun_array_int_destroy(&array);
		(void)cleanup;
		return false;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_NO_ERROR(destroy_result);

	printf("PASSED\n");
	return true;
}

int main()
{
	printf("Running Collections Unit Tests\n");
	printf("==============================\n\n");

	bool all_passed = true;

	all_passed &= test_fun_array_int_create_positive_initial_capacity();
	all_passed &= test_fun_array_int_push_non_full_array();

	printf("\n==============================\n");
	if (all_passed) {
		printf("✓ ALL TESTS PASSED!\n");
		return 0;
	} else {
		printf("✗ SOME TESTS FAILED!\n");
		return 1;
	}
}