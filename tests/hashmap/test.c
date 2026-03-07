#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/hashmap/hashmap.h"

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define ASSERT_ERROR_OK(result) assert((result).code == 0)
#define ASSERT_RESULT_OK(result) assert((result).error.code == 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

// Simple int hash and equals functions
static inline uint64_t hash_int(const void *key)
{
	return (uint64_t)(*(const int *)key);
}

static inline bool equals_int(const void *k1, const void *k2)
{
	return *(const int *)k1 == *(const int *)k2;
}

void test_fun_hashmap_create(void)
{
	HashMapResult result =
		fun_hashmap_create(sizeof(int), sizeof(int), 16, hash_int, equals_int);
	ASSERT_RESULT_OK(result);
	assert(fun_hashmap_size(&result.value) == 0);

	ErrorResult destroy = fun_hashmap_destroy(&result.value);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_create");
}

void test_fun_hashmap_put_get(void)
{
	HashMapResult create_result =
		fun_hashmap_create(sizeof(int), sizeof(int), 16, hash_int, equals_int);
	ASSERT_RESULT_OK(create_result);

	HashMap map = create_result.value;

	// Insert some key-value pairs
	int key1 = 1, val1 = 100;
	int key2 = 2, val2 = 200;
	int key3 = 3, val3 = 300;

	ASSERT_ERROR_OK(fun_hashmap_put(&map, &key1, &val1));
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &key2, &val2));
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &key3, &val3));

	// Retrieve them
	int out_val;
	ASSERT_ERROR_OK(fun_hashmap_get(&map, &key1, &out_val));
	assert(out_val == 100);

	ASSERT_ERROR_OK(fun_hashmap_get(&map, &key2, &out_val));
	assert(out_val == 200);

	ASSERT_ERROR_OK(fun_hashmap_get(&map, &key3, &out_val));
	assert(out_val == 300);

	ErrorResult destroy = fun_hashmap_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_put_get");
}

void test_fun_hashmap_update_existing_key(void)
{
	HashMapResult create_result =
		fun_hashmap_create(sizeof(int), sizeof(int), 16, hash_int, equals_int);
	ASSERT_RESULT_OK(create_result);

	HashMap map = create_result.value;

	// Insert initial value
	int key = 1, val = 100;
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &key, &val));

	int out_val;
	ASSERT_ERROR_OK(fun_hashmap_get(&map, &key, &out_val));
	assert(out_val == 100);

	// Update the value
	int new_val = 999;
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &key, &new_val));

	ASSERT_ERROR_OK(fun_hashmap_get(&map, &key, &out_val));
	assert(out_val == 999);

	// Size should remain the same (not a new entry)
	assert(fun_hashmap_size(&map) == 1);

	ErrorResult destroy = fun_hashmap_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_update_existing_key");
}

void test_fun_hashmap_get_nonexistent_key(void)
{
	HashMapResult create_result =
		fun_hashmap_create(sizeof(int), sizeof(int), 16, hash_int, equals_int);
	ASSERT_RESULT_OK(create_result);

	HashMap map = create_result.value;

	// Insert one value
	int key = 1, val = 100;
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &key, &val));

	// Try to get a key that doesn't exist
	int nonexistent_key = 999;
	int out_val;
	ErrorResult get_result = fun_hashmap_get(&map, &nonexistent_key, &out_val);
	assert(fun_error_is_error(get_result)); // Should error

	ErrorResult destroy = fun_hashmap_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_get_nonexistent_key");
}

void test_fun_hashmap_contains(void)
{
	HashMapResult create_result =
		fun_hashmap_create(sizeof(int), sizeof(int), 16, hash_int, equals_int);
	ASSERT_RESULT_OK(create_result);

	HashMap map = create_result.value;

	int key = 42, val = 4200;
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &key, &val));

	bool contains_42 = false;
	ASSERT_ERROR_OK(fun_hashmap_contains(&map, &key, &contains_42));
	assert(contains_42 == true);

	int nonexistent_key = 99;
	bool contains_99 = false;
	ASSERT_ERROR_OK(fun_hashmap_contains(&map, &nonexistent_key, &contains_99));
	assert(contains_99 == false);

	ErrorResult destroy = fun_hashmap_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_contains");
}

void test_fun_hashmap_remove(void)
{
	HashMapResult create_result =
		fun_hashmap_create(sizeof(int), sizeof(int), 16, hash_int, equals_int);
	ASSERT_RESULT_OK(create_result);

	HashMap map = create_result.value;

	int k1 = 1, v1 = 100;
	int k2 = 2, v2 = 200;
	int k3 = 3, v3 = 300;

	ASSERT_ERROR_OK(fun_hashmap_put(&map, &k1, &v1));
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &k2, &v2));
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &k3, &v3));

	assert(fun_hashmap_size(&map) == 3);

	// Remove middle element
	ASSERT_ERROR_OK(fun_hashmap_remove(&map, &k2));
	assert(fun_hashmap_size(&map) == 2);

	// Verify others still exist
	int out_val;
	ASSERT_ERROR_OK(fun_hashmap_get(&map, &k1, &out_val));
	assert(out_val == 100);

	ASSERT_ERROR_OK(fun_hashmap_get(&map, &k3, &out_val));
	assert(out_val == 300);

	// Try to remove non-existent key
	int nonexistent_key = 999;
	ErrorResult remove_result = fun_hashmap_remove(&map, &nonexistent_key);
	assert(fun_error_is_error(remove_result));

	ErrorResult destroy = fun_hashmap_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_remove");
}

void test_fun_hashmap_collision_handling(void)
{
	HashMapResult create_result =
		fun_hashmap_create(sizeof(int), sizeof(int), 4, hash_int, equals_int);
	ASSERT_RESULT_OK(create_result);

	HashMap map = create_result.value;

	// Insert many values to force collisions
	for (int i = 0; i < 20; i++) {
		int key = i, val = i * 100;
		ASSERT_ERROR_OK(fun_hashmap_put(&map, &key, &val));
	}

	// Verify all values retrievable
	for (int i = 0; i < 20; i++) {
		int key = i;
		int out_val;
		ASSERT_ERROR_OK(fun_hashmap_get(&map, &key, &out_val));
		assert(out_val == i * 100);
	}

	assert(fun_hashmap_size(&map) == 20);

	ErrorResult destroy = fun_hashmap_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_collision_handling");
}

void test_fun_hashmap_size_queries(void)
{
	HashMapResult create_result =
		fun_hashmap_create(sizeof(int), sizeof(int), 16, hash_int, equals_int);
	ASSERT_RESULT_OK(create_result);

	HashMap map = create_result.value;

	assert(fun_hashmap_size(&map) == 0);

	int k1 = 1, v1 = 100;
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &k1, &v1));
	assert(fun_hashmap_size(&map) == 1);

	int k2 = 2, v2 = 200;
	ASSERT_ERROR_OK(fun_hashmap_put(&map, &k2, &v2));
	assert(fun_hashmap_size(&map) == 2);

	ASSERT_ERROR_OK(fun_hashmap_remove(&map, &k1));
	assert(fun_hashmap_size(&map) == 1);

	ErrorResult destroy = fun_hashmap_destroy(&map);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_hashmap_size_queries");
}

void test_fun_hashmap_memory_leak_prevention(void)
{
	// Create and destroy multiple hashmaps
	for (int i = 0; i < 10; i++) {
		HashMapResult result = fun_hashmap_create(sizeof(int), sizeof(int), 8,
												  hash_int, equals_int);
		ASSERT_RESULT_OK(result);

		for (int j = 0; j < 5; j++) {
			int key = j, val = j * 10;
			ASSERT_ERROR_OK(fun_hashmap_put(&result.value, &key, &val));
		}

		ErrorResult destroy = fun_hashmap_destroy(&result.value);
		ASSERT_ERROR_OK(destroy);
	}

	print_test_result("fun_hashmap_memory_leak_prevention");
}

int main(void)
{
	printf("Running HashMap Module Unit Tests\n");
	printf("=================================\n\n");

	test_fun_hashmap_create();
	test_fun_hashmap_put_get();
	test_fun_hashmap_update_existing_key();
	test_fun_hashmap_get_nonexistent_key();
	test_fun_hashmap_contains();
	test_fun_hashmap_remove();
	test_fun_hashmap_collision_handling();
	test_fun_hashmap_size_queries();
	test_fun_hashmap_memory_leak_prevention();

	printf("\n=================================\n");
	printf("All hashmap tests completed\n");
	return 0;
}
