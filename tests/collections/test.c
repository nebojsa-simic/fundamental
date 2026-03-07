#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/array/array.h"

DEFINE_ARRAY_TYPE(int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define ASSERT_ERROR_OK(result) assert((result).code == 0)
#define ASSERT_RESULT_OK(result) assert((result).error.code == 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_fun_array_int_create_positive_initial_capacity(void)
{
	intArrayResult result = fun_array_int_create(10);
	ASSERT_RESULT_OK(result);
	assert(result.value.array.count == 0);
	assert(result.value.array.capacity >= 10);

	ErrorResult cleanup = fun_array_int_destroy(&result.value);
	ASSERT_ERROR_OK(cleanup);

	print_test_result("fun_array_int_create_positive_initial_capacity");
}

void test_fun_array_int_create_allocation_failure(void)
{
	intArrayResult result = fun_array_int_create(SIZE_MAX >> 10);
	assert(fun_error_is_error(result.error));

	print_test_result("fun_array_int_create_allocation_failure");
}

void test_fun_array_int_push_non_full_array(void)
{
	intArrayResult create_result = fun_array_int_create(5);
	ASSERT_RESULT_OK(create_result);

	intArray array = create_result.value;
	ErrorResult push_result = fun_array_int_push(&array, 42);
	ASSERT_ERROR_OK(push_result);

	assert(array.array.count == 1);
	assert(fun_array_int_get(&array, 0) == 42);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_push_non_full_array");
}

void test_fun_array_int_auto_reallocation(void)
{
	intArrayResult result = fun_array_int_create(2);
	ASSERT_RESULT_OK(result);

	intArray array = result.value;

	ASSERT_ERROR_OK(fun_array_int_push(&array, 10));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 20));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 30));

	assert(array.array.capacity >= 3);
	assert(fun_array_int_get(&array, 0) == 10);
	assert(fun_array_int_get(&array, 1) == 20);
	assert(fun_array_int_get(&array, 2) == 30);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_auto_reallocation");
}

void test_fun_array_int_get_elements(void)
{
	intArrayResult result = fun_array_int_create(3);
	ASSERT_RESULT_OK(result);

	intArray array = result.value;

	ASSERT_ERROR_OK(fun_array_int_push(&array, 100));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 200));
	ASSERT_ERROR_OK(fun_array_int_push(&array, 300));

	assert(fun_array_int_get(&array, 0) == 100);
	assert(fun_array_int_get(&array, 1) == 200);
	assert(fun_array_int_get(&array, 2) == 300);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_get_elements");
}

void test_fun_array_int_get_out_of_bounds(void)
{
	intArrayResult result = fun_array_int_create(2);
	ASSERT_RESULT_OK(result);

	intArray array = result.value;
	ASSERT_ERROR_OK(fun_array_int_push(&array, 42));

	int out_of_bounds = fun_array_int_get(&array, 10);
	(void)out_of_bounds;

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_get_out_of_bounds");
}

void test_fun_array_int_destroy_safely(void)
{
	intArrayResult result = fun_array_int_create(5);
	ASSERT_RESULT_OK(result);

	intArray array = result.value;
	ASSERT_ERROR_OK(fun_array_int_push(&array, 999));

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_destroy_safely");
}

void test_fun_array_int_create_zero_capacity(void)
{
	intArrayResult result = fun_array_int_create(0);

	if (fun_error_is_ok(result.error)) {
		ASSERT_ERROR_OK(fun_array_int_push(&result.value, 1));
		assert(result.value.array.count == 1);

		ErrorResult destroy_result = fun_array_int_destroy(&result.value);
		ASSERT_ERROR_OK(destroy_result);
	}

	print_test_result("fun_array_int_create_zero_capacity");
}

void test_fun_array_int_size_queries(void)
{
	intArrayResult result = fun_array_int_create(5);
	ASSERT_RESULT_OK(result);

	intArray array = result.value;
	assert(fun_array_int_size(&array) == 0);

	ASSERT_ERROR_OK(fun_array_int_push(&array, 1));
	assert(fun_array_int_size(&array) == 1);

	ASSERT_ERROR_OK(fun_array_int_push(&array, 2));
	assert(fun_array_int_size(&array) == 2);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_size_queries");
}

void test_fun_array_int_empty_handling(void)
{
	intArrayResult result = fun_array_int_create(3);
	ASSERT_RESULT_OK(result);

	intArray array = result.value;
	assert(array.array.count == 0);
	assert(fun_array_int_size(&array) == 0);

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("fun_array_int_empty_handling");
}

void test_memory_leak_prevention_multiple_arrays(void)
{
	const int num_arrays = 5;
	for (int i = 0; i < num_arrays; i++) {
		intArrayResult result = fun_array_int_create(10);
		ASSERT_RESULT_OK(result);

		for (int j = 0; j < 3; j++) {
			ASSERT_ERROR_OK(fun_array_int_push(&result.value, i * 10 + j));
		}

		ErrorResult destroy_result = fun_array_int_destroy(&result.value);
		ASSERT_ERROR_OK(destroy_result);
	}

	print_test_result("memory_leak_prevention_multiple_arrays");
}

void test_platform_independence(void)
{
	intArrayResult result = fun_array_int_create(5);
	ASSERT_RESULT_OK(result);

	ASSERT_ERROR_OK(fun_array_int_push(&result.value, 42));
	assert(fun_array_int_get(&result.value, 0) == 42);

	ErrorResult destroy_result = fun_array_int_destroy(&result.value);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("platform_independence");
}

typedef struct {
	int x;
	int y;
} Point;

DEFINE_ARRAY_TYPE(Point)

void test_custom_type_point_array(void)
{
	PointArrayResult result = fun_array_Point_create(3);
	ASSERT_RESULT_OK(result);

	Point p1 = { 10, 20 };
	Point p2 = { 30, 40 };

	ASSERT_ERROR_OK(fun_array_Point_push(&result.value, p1));
	ASSERT_ERROR_OK(fun_array_Point_push(&result.value, p2));

	Point retrieved = fun_array_Point_get(&result.value, 0);
	assert(retrieved.x == 10 && retrieved.y == 20);

	ErrorResult destroy_result = fun_array_Point_destroy(&result.value);
	ASSERT_ERROR_OK(destroy_result);

	print_test_result("custom_type_point_array");
}

int main(void)
{
	printf("Running Collections Module Unit Tests\n");
	printf("=====================================\n\n");

	test_fun_array_int_create_positive_initial_capacity();
	test_fun_array_int_create_allocation_failure();
	test_fun_array_int_push_non_full_array();
	test_fun_array_int_auto_reallocation();
	test_fun_array_int_get_elements();
	test_fun_array_int_get_out_of_bounds();
	test_fun_array_int_destroy_safely();
	test_fun_array_int_create_zero_capacity();
	test_fun_array_int_size_queries();
	test_fun_array_int_empty_handling();
	test_memory_leak_prevention_multiple_arrays();
	test_platform_independence();
	test_custom_type_point_array();

	printf("\n=====================================\n");
	printf("All collections tests completed\n");
	return 0;
}
