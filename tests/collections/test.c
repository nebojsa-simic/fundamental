#include "fundamental/array/array.h"
#include "fundamental/console/console.h"

DEFINE_ARRAY_TYPE(int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

void test_fun_array_int_create_positive_initial_capacity(void)
{
	intArrayResult result = fun_array_int_create(10);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}
	if (!(result.value.array.count == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(result.value.array.capacity >= 10)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult cleanup = fun_array_int_destroy(&result.value);
	if (cleanup.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_create_positive_initial_capacity");
}

void test_fun_array_int_create_allocation_failure(void)
{
	intArrayResult result = fun_array_int_create(SIZE_MAX >> 10);
	if (!(fun_error_is_error(result.error))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	print_test_result("fun_array_int_create_allocation_failure");
}

void test_fun_array_int_push_non_full_array(void)
{
	intArrayResult create_result = fun_array_int_create(5);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intArray array = create_result.value;
	ErrorResult push_result = fun_array_int_push(&array, 42);
	if (push_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(array.array.count == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_array_int_get(&array, 0) == 42)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_push_non_full_array");
}

void test_fun_array_int_auto_reallocation(void)
{
	intArrayResult result = fun_array_int_create(2);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intArray array = result.value;

	if (fun_array_int_push(&array, 10).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_array_int_push(&array, 20).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_array_int_push(&array, 30).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(array.array.capacity >= 3)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_array_int_get(&array, 0) == 10)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_array_int_get(&array, 1) == 20)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_array_int_get(&array, 2) == 30)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_auto_reallocation");
}

void test_fun_array_int_get_elements(void)
{
	intArrayResult result = fun_array_int_create(3);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intArray array = result.value;

	if (fun_array_int_push(&array, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_array_int_push(&array, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_array_int_push(&array, 300).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(fun_array_int_get(&array, 0) == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_array_int_get(&array, 1) == 200)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_array_int_get(&array, 2) == 300)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_get_elements");
}

void test_fun_array_int_get_out_of_bounds(void)
{
	intArrayResult result = fun_array_int_create(2);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intArray array = result.value;
	if (fun_array_int_push(&array, 42).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	int out_of_bounds = fun_array_int_get(&array, 10);
	(void)out_of_bounds;

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_get_out_of_bounds");
}

void test_fun_array_int_destroy_safely(void)
{
	intArrayResult result = fun_array_int_create(5);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intArray array = result.value;
	if (fun_array_int_push(&array, 999).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_destroy_safely");
}

void test_fun_array_int_create_zero_capacity(void)
{
	intArrayResult result = fun_array_int_create(0);

	if (fun_error_is_ok(result.error)) {
		if (fun_array_int_push(&result.value, 1).code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
		if (!(result.value.array.count == 1)) {
			fun_console_write_line("FAIL: assertion");
			return;
		}

		ErrorResult destroy_result = fun_array_int_destroy(&result.value);
		if (destroy_result.code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
	}

	print_test_result("fun_array_int_create_zero_capacity");
}

void test_fun_array_int_size_queries(void)
{
	intArrayResult result = fun_array_int_create(5);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intArray array = result.value;
	if (!(fun_array_int_size(&array) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_array_int_push(&array, 1).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_array_int_size(&array) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_array_int_push(&array, 2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_array_int_size(&array) == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_size_queries");
}

void test_fun_array_int_empty_handling(void)
{
	intArrayResult result = fun_array_int_create(3);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intArray array = result.value;
	if (!(array.array.count == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_array_int_size(&array) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&array);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_array_int_empty_handling");
}

void test_memory_leak_prevention_multiple_arrays(void)
{
	const int num_arrays = 5;
	for (int i = 0; i < num_arrays; i++) {
		intArrayResult result = fun_array_int_create(10);
		if (result.error.code != 0) {
			fun_console_write_line("FAIL: ASSERT_RESULT_OK");
			return;
		}

		for (int j = 0; j < 3; j++) {
			if (fun_array_int_push(&result.value, i * 10 + j).code != 0) {
				fun_console_write_line("FAIL: ASSERT_ERROR_OK");
				return;
			}
		}

		ErrorResult destroy_result = fun_array_int_destroy(&result.value);
		if (destroy_result.code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
	}

	print_test_result("memory_leak_prevention_multiple_arrays");
}

void test_platform_independence(void)
{
	intArrayResult result = fun_array_int_create(5);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	if (fun_array_int_push(&result.value, 42).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_array_int_get(&result.value, 0) == 42)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy_result = fun_array_int_destroy(&result.value);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

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
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	Point p1 = { 10, 20 };
	Point p2 = { 30, 40 };

	if (fun_array_Point_push(&result.value, p1).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_array_Point_push(&result.value, p2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	Point retrieved = fun_array_Point_get(&result.value, 0);
	if (!(retrieved.x == 10 && retrieved.y == 20)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy_result = fun_array_Point_destroy(&result.value);
	if (destroy_result.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("custom_type_point_array");
}

int main(void)
{
	fun_console_write_line("Running Collections Module Unit Tests");
	fun_console_write_line("=====================================");
	fun_console_write_line("");

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

	fun_console_write_line("");
	fun_console_write_line("=====================================");
	fun_console_write_line("All collections tests completed");
	return 0;
}
