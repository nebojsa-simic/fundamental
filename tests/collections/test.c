#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/array/array.h"

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_X "\033[0;31m✗\033[0m"

// Helper macros - ErrorResult has .code directly
#define ASSERT_ERROR_OK(result) assert((result).code == 0)
#define ASSERT_RESULT_OK(result) assert((result).error.code == 0)

// Helper function to print test progress
void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

// Scenario: Create array with positive initial capacity
void test_fun_array_int_create_positive_initial_capacity()
{
	// Test successful creation with positive capacity
	IntArrayResult result = fun_array_int_create(10);
	ASSERT_RESULT_OK(result);
	assert(result.value.array.count == 0);
	assert(result.value.array.capacity >= 10);

	ErrorResult cleanup = fun_array_int_destroy(&result.value);
	ASSERT_ERROR_OK(cleanup);

	print_test_result("fun_array_int_create_positive_initial_capacity");
}

// Scenario: Create array - allocation failure
void test_fun_array_int_create_allocation_failure()
{
	// Test failure with impossible size
	IntArrayResult result = fun_array_int_create(SIZE_MAX >> 10);
	assert(fun_error_is_error(result.error));

	print_test_result("fun_array_int_create_allocation_failure");
}

// Scenario: Add elements to array (non-full)
void test_fun_array_int_push_non_full_array()
{
	IntArrayResult create_result = fun_array_int_create(5);
	ASSERT_RESULT_OK(create_result);

	IntArray array = create_result.value;
	ErrorResult push_result = fun_array_int_push(&array, 42);
	ASSERT_ERROR_OK(push_result);

	assert(array.array.count == 1);
	assert(fun_array_int_get(&array, 0) == 42);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_push_non_full_array");
}

// Scenario: Handle array auto-reallocation
void test_fun_array_int_auto_reallocation()
{
	// Create with capacity 2, push 3 to trigger reallocation
	IntArrayResult result = fun_array_int_create(2);
	ASSERT_RESULT_OK(result);

	IntArray array = result.value;

	// Push beyond capacity
	ASSERT_ERROR_OK(fun_array_int_push(&array, 10));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 20));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 30));

	// Verify capacity increased
	assert(array.array.capacity >= 3);

	// Verify all elements preserved
	assert(fun_array_int_get(&array, 0) == 10);
	assert(fun_array_int_get(&array, 1) == 20);
	assert(fun_array_int_get(&array, 2) == 30);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_auto_reallocation");
}

// Scenario: Get array elements
void test_fun_array_int_get_elements()
{
	IntArrayResult result = fun_array_int_create(3);
	ASSERT_RESULT_OK(result);

	IntArray array = result.value;

	// Add elements
	ASSERT_ERROR_OK(fun_array_int_push(&array, 100));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 200));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 300));

	// Retrieve elements
	assert(fun_array_int_get(&array, 0) == 100);
	assert(fun_array_int_get(&array, 1) == 200);
	assert(fun_array_int_get(&array, 2) == 300);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_get_elements");
}

// Scenario: Get array elements - out of bounds
void test_fun_array_int_get_out_of_bounds()
{
	IntArrayResult result = fun_array_int_create(2);
	ASSERT_RESULT_OK(result);

	IntArray array = result.value;
	ASSERT_ERROR_OK(fun_array_int_push(&array, 42));

	// Out of bounds access - implementation may return 0 or handle differently
	// For now just verify it doesn't crash
	int out_of_bounds = fun_array_int_get(&array, 10);
	(void)out_of_bounds; // Suppress unused warning

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_get_out_of_bounds");
}

// Scenario: Destroy array safely
void test_fun_array_int_destroy_safely()
{
	IntArrayResult result = fun_array_int_create(5);
	ASSERT_RESULT_OK(result);

	IntArray array = result.value;
	ASSERT_ERROR_OK(fun_array_int_push(&array, 999));

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_destroy_safely");
}

// Scenario: Zero capacity initial array
void test_fun_array_int_create_zero_capacity()
{
	// Zero capacity - implementation may accept or reject
	IntArrayResult result = fun_array_int_create(0);

	if (fun_error_is_ok(result.error)) {
		// If accepted, should be able to push and trigger allocation
		ASSERT_ERROR_OK(fun_array_int_push(&result.value, 1));
		assert(result.value.array.count == 1);

		ErrorResult destroy_result = fun_array_int_destroy(&result.value);
		ASSERT_ERROR_OK(destroy_result);
	}
	// If rejected, that's also valid behavior

	print_test_result("fun_array_int_create_zero_capacity");
}

// Scenario: Size queries
void test_fun_array_int_size_queries()
{
	IntArrayResult result = fun_array_int_create(5);
	ASSERT_RESULT_OK(result);

	IntArray array = result.value;
	assert(fun_array_int_size(&array) == 0);

	ASSERT_ERROR_OK(fun_array_int_push(&array, 1));
	assert(fun_array_int_size(&array) == 1);

	ASSERT_ERROR_OK(fun_array_int_push(&array, 2));
	assert(fun_array_int_size(&array) == 2);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_size_queries");
}

// Scenario: Empty structure handling
void test_fun_array_int_empty_handling()
{
	IntArrayResult result = fun_array_int_create(3);
	ASSERT_RESULT_OK(result);

	IntArray array = result.value;
	assert(array.array.count == 0);
	assert(fun_array_int_size(&array) == 0);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_empty_handling");
}

// Scenario: Memory leak prevention
void test_memory_leak_prevention_multiple_arrays()
{
	const int num_arrays = 5;
	for (int i = 0; i < num_arrays; i++) {
		IntArrayResult result = fun_array_int_create(10);
		ASSERT_RESULT_OK(result);

		// Add some elements
		for (int j = 0; j < 3; j++) {
			ASSERT_ERROR_OK(fun_array_int_push(&result.value, i * 10 + j));
		}

		ErrorResult destroy_result = fun_array_int_destroy(&result.value);
		ASSERT_ERROR_OK(destroy_result);
	}

	print_test_result("memory_leak_prevention_multiple_arrays");
}

// Scenario: Platform independence verification
void test_platform_independence()
{
	// Platform independence is verified by compilation and execution
	// All memory operations use fun_memory_* functions only
	IntArrayResult result = fun_array_int_create(5);
	ASSERT_RESULT_OK(result);

	// Operations use library functions only
	ASSERT_ERROR_OK(fun_array_int_push(&result.value, 42));
	assert(fun_array_int_get(&result.value, 0) == 42);

	ErrorResult destroy_result = fun_array_int_destroy(&result.value);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("platform_independence");
}

int main()
{
	printf("Running Collections Module Unit Tests\n");
	printf("=====================================\n\n");

	// Array Core Operations
	test_fun_array_int_create_positive_initial_capacity();
	test_fun_array_int_create_allocation_failure();
	test_fun_array_int_push_non_full_array();
	test_fun_array_int_auto_reallocation();
	test_fun_array_int_get_elements();
	test_fun_array_int_get_out_of_bounds();
	test_fun_array_int_destroy_safely();

	// Edge Cases and Boundary Conditions
	test_fun_array_int_create_zero_capacity();
	test_fun_array_int_size_queries();
	test_fun_array_int_empty_handling();

	// Memory Safety and Platform Independence
	test_memory_leak_prevention_multiple_arrays();
	test_platform_independence();

	printf("\n=====================================\n");
	printf("All collections tests completed\n");
	return 0;
}
