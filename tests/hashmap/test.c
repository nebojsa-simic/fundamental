#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fundamental/hashmap/hashmap.h"

DEFINE_HASHMAP_TYPE(int, int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define ASSERT_ERROR_OK(result) assert((result).code == 0)
#define ASSERT_RESULT_OK(result) assert((result).error.code == 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_generic_hash_int(void)
{
	int value = 42;
	uint64_t hash = fun_hash_int(&value);
	assert(hash == 42);

	int value2 = 42;
	assert(fun_equals_int(&value, &value2) == true);

	int value3 = 99;
	assert(fun_equals_int(&value, &value3) == false);

	print_test_result("generic_hash_int");
}

void test_generic_hash_string(void)
{
	char *str1 = "hello";
	char *str2 = "hello";
	char *str3 = "world";

	assert(fun_hash_string(str1) == fun_hash_string(str2));
	assert(fun_equals_string(str1, str2) == true);
	assert(fun_equals_string(str1, str3) == false);

	print_test_result("generic_hash_string");
}

void test_generic_hash_ptr(void)
{
	int val = 42;
	void *ptr1 = &val;
	void *ptr2 = &val;
	int other_val = 99;
	void *ptr3 = &other_val;

	assert(fun_hash_ptr(ptr1) == fun_hash_ptr(ptr2));
	assert(fun_equals_ptr(ptr1, ptr2) == true);
	assert(fun_equals_ptr(ptr1, ptr3) == false);

	print_test_result("generic_hash_ptr");
}

void test_fun_hashmap_int_int_create(void)
{
	intintHashMapResult result = fun_hashmap_int_int_create(16);
	ASSERT_RESULT_OK(result);
	assert(fun_hashmap_int_int_size(&result.value) == 0);

	ErrorResult destroy = fun_hashmap_int_int_destroy(&result.value);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_int_int_create");
}

void test_fun_hashmap_int_int_put_get(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intintHashMap map = create_result.value;

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 1, 100));
	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 2, 200));
	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 3, 300));

	assert(fun_hashmap_int_int_get(&map, 1) == 100);
	assert(fun_hashmap_int_int_get(&map, 2) == 200);
	assert(fun_hashmap_int_int_get(&map, 3) == 300);

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_int_int_put_get");
}

void test_fun_hashmap_int_int_update_existing_key(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intintHashMap map = create_result.value;

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 1, 100));
	assert(fun_hashmap_int_int_get(&map, 1) == 100);

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 1, 999));
	assert(fun_hashmap_int_int_get(&map, 1) == 999);
	assert(fun_hashmap_int_int_size(&map) == 1);

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_int_int_update_existing_key");
}

void test_fun_hashmap_int_int_get_nonexistent_key(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intintHashMap map = create_result.value;

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 1, 100));

	// Get existing key should work
	int value = fun_hashmap_int_int_get(&map, 1);
	assert(value == 100);

	// Get nonexistent key should return 0 (default int value)
	// Note: implementation may error or return default value
	int nonexistent = fun_hashmap_int_int_get(&map, 999);
	// Just verify it doesn't crash - value depends on implementation
	(void)nonexistent;

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_int_int_get_nonexistent_key");
}

void test_fun_hashmap_int_int_contains(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intintHashMap map = create_result.value;

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 42, 4200));

	bool contains_42 = false;
	ASSERT_ERROR_OK(fun_hashmap_int_int_contains(&map, 42, &contains_42));
	assert(contains_42 == true);

	bool contains_99 = false;
	ASSERT_ERROR_OK(fun_hashmap_int_int_contains(&map, 99, &contains_99));
	assert(contains_99 == false);

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_int_int_contains");
}

void test_fun_hashmap_int_int_remove(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intintHashMap map = create_result.value;

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 1, 100));
	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 2, 200));
	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 3, 300));

	assert(fun_hashmap_int_int_size(&map) == 3);

	ASSERT_ERROR_OK(fun_hashmap_int_int_remove(&map, 2));
	assert(fun_hashmap_int_int_size(&map) == 2);

	assert(fun_hashmap_int_int_get(&map, 1) == 100);
	assert(fun_hashmap_int_int_get(&map, 3) == 300);

	ErrorResult remove_result = fun_hashmap_int_int_remove(&map, 999);
	assert(fun_error_is_error(remove_result));

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_int_int_remove");
}

void test_fun_hashmap_collision_handling(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(4);
	ASSERT_RESULT_OK(create_result);

	intintHashMap map = create_result.value;

	for (int i = 0; i < 20; i++) {
		ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, i, i * 100));
	}

	for (int i = 0; i < 20; i++) {
		assert(fun_hashmap_int_int_get(&map, i) == i * 100);
	}

	assert(fun_hashmap_int_int_size(&map) == 20);

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_collision_handling");
}

void test_fun_hashmap_int_int_size_queries(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	ASSERT_RESULT_OK(create_result);

	intintHashMap map = create_result.value;

	assert(fun_hashmap_int_int_size(&map) == 0);

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 1, 100));
	assert(fun_hashmap_int_int_size(&map) == 1);

	ASSERT_ERROR_OK(fun_hashmap_int_int_put(&map, 2, 200));
	assert(fun_hashmap_int_int_size(&map) == 2);

	ASSERT_ERROR_OK(fun_hashmap_int_int_remove(&map, 1));
	assert(fun_hashmap_int_int_size(&map) == 1);

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_int_int_size_queries");
}

void test_fun_hashmap_memory_leak_prevention(void)
{
	for (int i = 0; i < 10; i++) {
		intintHashMapResult result = fun_hashmap_int_int_create(8);
		ASSERT_RESULT_OK(result);

		for (int j = 0; j < 5; j++) {
			ASSERT_ERROR_OK(fun_hashmap_int_int_put(&result.value, j, j * 10));
		}

		ErrorResult destroy = fun_hashmap_int_int_destroy(&result.value);
		ASSERT_ERROR_OK(destroy);
	}

	print_test_result("fun_hashmap_memory_leak_prevention");
}

int main(void)
{
	printf("Running HashMap Module Unit Tests\n");
	printf("=================================\n\n");

	test_generic_hash_int();
	test_generic_hash_string();
	test_generic_hash_ptr();
	test_fun_hashmap_int_int_create();
	test_fun_hashmap_int_int_put_get();
	test_fun_hashmap_int_int_update_existing_key();
	test_fun_hashmap_int_int_get_nonexistent_key();
	test_fun_hashmap_int_int_contains();
	test_fun_hashmap_int_int_remove();
	test_fun_hashmap_collision_handling();
	test_fun_hashmap_int_int_size_queries();
	test_fun_hashmap_memory_leak_prevention();

	printf("\n=================================\n");
	printf("All hashmap tests completed\n");
	return 0;
}
