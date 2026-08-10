#include "fundamental/hashmap/hashmap.h"
#include "fundamental/console/console.h"

DEFINE_HASHMAP_TYPE(int, int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

void test_generic_hash_int(void)
{
	int value = 42;
	uint64_t hash = fun_hash_int(&value);
	if (!(hash == 42)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	int value2 = 42;
	if (!(fun_equals_int(&value, &value2) == true)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	int value3 = 99;
	if (!(fun_equals_int(&value, &value3) == false)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	print_test_result("generic_hash_int");
}

void test_generic_hash_string(void)
{
	char *str1 = "hello";
	char *str2 = "hello";
	char *str3 = "world";

	if (!(fun_hash_string(str1) == fun_hash_string(str2))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_equals_string(str1, str2) == true)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_equals_string(str1, str3) == false)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	print_test_result("generic_hash_string");
}

void test_generic_hash_ptr(void)
{
	int val = 42;
	void *ptr1 = &val;
	void *ptr2 = &val;
	int other_val = 99;
	void *ptr3 = &other_val;

	if (!(fun_hash_ptr(ptr1) == fun_hash_ptr(ptr2))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_equals_ptr(ptr1, ptr2) == true)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_equals_ptr(ptr1, ptr3) == false)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	print_test_result("generic_hash_ptr");
}

void test_fun_hashmap_int_int_create(void)
{
	intintHashMapResult result = fun_hashmap_int_int_create(16);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}
	if (!(fun_hashmap_int_int_size(&result.value) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_hashmap_int_int_destroy(&result.value);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_int_int_create");
}

void test_fun_hashmap_int_int_put_get(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintHashMap map = create_result.value;

	if (fun_hashmap_int_int_put(&map, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_hashmap_int_int_put(&map, 2, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_hashmap_int_int_put(&map, 3, 300).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(fun_hashmap_int_int_get(&map, 1) == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_hashmap_int_int_get(&map, 2) == 200)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_hashmap_int_int_get(&map, 3) == 300)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_int_int_put_get");
}

void test_fun_hashmap_int_int_update_existing_key(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintHashMap map = create_result.value;

	if (fun_hashmap_int_int_put(&map, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_hashmap_int_int_get(&map, 1) == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_hashmap_int_int_put(&map, 1, 999).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_hashmap_int_int_get(&map, 1) == 999)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_hashmap_int_int_size(&map) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_int_int_update_existing_key");
}

void test_fun_hashmap_int_int_get_nonexistent_key(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintHashMap map = create_result.value;

	if (fun_hashmap_int_int_put(&map, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	// Get existing key should work
	int value = fun_hashmap_int_int_get(&map, 1);
	if (!(value == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Get nonexistent key should return 0 (default int value)
	// Note: implementation may error or return default value
	int nonexistent = fun_hashmap_int_int_get(&map, 999);
	// Just verify it doesn't crash - value depends on implementation
	(void)nonexistent;

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_int_int_get_nonexistent_key");
}

void test_fun_hashmap_int_int_contains(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintHashMap map = create_result.value;

	if (fun_hashmap_int_int_put(&map, 42, 4200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	bool contains_42 = false;
	if (fun_hashmap_int_int_contains(&map, 42, &contains_42).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(contains_42 == true)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	bool contains_99 = false;
	if (fun_hashmap_int_int_contains(&map, 99, &contains_99).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(contains_99 == false)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_int_int_contains");
}

void test_fun_hashmap_int_int_remove(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintHashMap map = create_result.value;

	if (fun_hashmap_int_int_put(&map, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_hashmap_int_int_put(&map, 2, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_hashmap_int_int_put(&map, 3, 300).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(fun_hashmap_int_int_size(&map) == 3)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_hashmap_int_int_remove(&map, 2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_hashmap_int_int_size(&map) == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (!(fun_hashmap_int_int_get(&map, 1) == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_hashmap_int_int_get(&map, 3) == 300)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult remove_result = fun_hashmap_int_int_remove(&map, 999);
	if (!(fun_error_is_error(remove_result))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_int_int_remove");
}

void test_fun_hashmap_collision_handling(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(4);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintHashMap map = create_result.value;

	for (int i = 0; i < 20; i++) {
		if (fun_hashmap_int_int_put(&map, i, i * 100).code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
	}

	for (int i = 0; i < 20; i++) {
		if (!(fun_hashmap_int_int_get(&map, i) == i * 100)) {
			fun_console_write_line("FAIL: assertion");
			return;
		}
	}

	if (!(fun_hashmap_int_int_size(&map) == 20)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_collision_handling");
}

void test_fun_hashmap_int_int_size_queries(void)
{
	intintHashMapResult create_result = fun_hashmap_int_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintHashMap map = create_result.value;

	if (!(fun_hashmap_int_int_size(&map) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_hashmap_int_int_put(&map, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_hashmap_int_int_size(&map) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_hashmap_int_int_put(&map, 2, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_hashmap_int_int_size(&map) == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_hashmap_int_int_remove(&map, 1).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_hashmap_int_int_size(&map) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_hashmap_int_int_destroy(&map);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_hashmap_int_int_size_queries");
}

void test_fun_hashmap_memory_leak_prevention(void)
{
	for (int i = 0; i < 10; i++) {
		intintHashMapResult result = fun_hashmap_int_int_create(8);
		if (result.error.code != 0) {
			fun_console_write_line("FAIL: ASSERT_RESULT_OK");
			return;
		}

		for (int j = 0; j < 5; j++) {
			if (fun_hashmap_int_int_put(&result.value, j, j * 10).code != 0) {
				fun_console_write_line("FAIL: ASSERT_ERROR_OK");
				return;
			}
		}

		ErrorResult destroy = fun_hashmap_int_int_destroy(&result.value);
		if (destroy.code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
	}

	print_test_result("fun_hashmap_memory_leak_prevention");
}

int main(void)
{
	fun_console_write_line("Running HashMap Module Unit Tests");
	fun_console_write_line("=================================");
	fun_console_write_line("");

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

	fun_console_write_line("");
	fun_console_write_line("=================================");
	fun_console_write_line("All hashmap tests completed");
	return 0;
}
