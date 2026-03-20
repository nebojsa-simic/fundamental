#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fundamental/set/set.h"

DEFINE_SET_TYPE(int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define ASSERT_ERROR_OK(result) assert((result).code == 0)
#define ASSERT_RESULT_OK(result) assert((result).error.code == 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_fun_set_int_add_contains(void)
{
	intSetResult create_result = fun_set_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intSet set = create_result.value;

	ASSERT_ERROR_OK(fun_set_int_add(&set, 10));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 20));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 30));

	bool contains_10 = false;
	ASSERT_ERROR_OK(fun_set_int_contains(&set, 10, &contains_10));
	assert(contains_10 == true);

	bool contains_99 = false;
	ASSERT_ERROR_OK(fun_set_int_contains(&set, 99, &contains_99));
	assert(contains_99 == false);

	ErrorResult destroy = fun_set_int_destroy(&set);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_set_int_add_contains");
}

void test_fun_set_int_no_duplicates(void)
{
	intSetResult create_result = fun_set_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intSet set = create_result.value;

	ASSERT_ERROR_OK(fun_set_int_add(&set, 42));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 42));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 42));

	// Size should still be 1 (no duplicates)
	assert(fun_set_int_size(&set) == 1);

	ErrorResult destroy = fun_set_int_destroy(&set);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_set_int_no_duplicates");
}

void test_fun_set_int_remove(void)
{
	intSetResult create_result = fun_set_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intSet set = create_result.value;

	ASSERT_ERROR_OK(fun_set_int_add(&set, 1));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 2));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 3));

	assert(fun_set_int_size(&set) == 3);

	ASSERT_ERROR_OK(fun_set_int_remove(&set, 2));
	assert(fun_set_int_size(&set) == 2);

	bool contains_2 = true;
	ASSERT_ERROR_OK(fun_set_int_contains(&set, 2, &contains_2));
	assert(contains_2 == false);

	ErrorResult remove_result = fun_set_int_remove(&set, 999);
	assert(fun_error_is_error(remove_result));

	ErrorResult destroy = fun_set_int_destroy(&set);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_set_int_remove");
}

void test_fun_set_int_size(void)
{
	intSetResult create_result = fun_set_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intSet set = create_result.value;

	assert(fun_set_int_size(&set) == 0);

	ASSERT_ERROR_OK(fun_set_int_add(&set, 1));
	assert(fun_set_int_size(&set) == 1);

	ASSERT_ERROR_OK(fun_set_int_add(&set, 2));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 3));
	ASSERT_ERROR_OK(fun_set_int_add(&set, 4));
	assert(fun_set_int_size(&set) == 4);

	ASSERT_ERROR_OK(fun_set_int_remove(&set, 2));
	assert(fun_set_int_size(&set) == 3);

	ErrorResult destroy = fun_set_int_destroy(&set);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_set_int_size");
}

void test_fun_set_int_many_elements(void)
{
	intSetResult create_result = fun_set_int_create(32);
	ASSERT_RESULT_OK(create_result);

	intSet set = create_result.value;

	// Add 100 elements
	for (int i = 0; i < 100; i++) {
		ASSERT_ERROR_OK(fun_set_int_add(&set, i));
	}

	assert(fun_set_int_size(&set) == 100);

	// Verify all elements present
	for (int i = 0; i < 100; i++) {
		bool contains = false;
		ASSERT_ERROR_OK(fun_set_int_contains(&set, i, &contains));
		assert(contains == true);
	}

	// Remove half
	for (int i = 0; i < 50; i++) {
		ASSERT_ERROR_OK(fun_set_int_remove(&set, i));
	}

	assert(fun_set_int_size(&set) == 50);

	ErrorResult destroy = fun_set_int_destroy(&set);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_set_int_many_elements");
}

int main(void)
{
	printf("Running Set Module Unit Tests\n");
	printf("==============================\n\n");

	test_fun_set_int_add_contains();
	test_fun_set_int_no_duplicates();
	test_fun_set_int_remove();
	test_fun_set_int_size();
	test_fun_set_int_many_elements();

	printf("\n==============================\n");
	printf("All set tests completed\n");
	return 0;
}
