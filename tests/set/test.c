#include "fundamental/set/set.h"
#include "fundamental/console/console.h"

DEFINE_SET_TYPE(int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

void test_fun_set_int_add_contains(void)
{
	intSetResult create_result = fun_set_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intSet set = create_result.value;

	if (fun_set_int_add(&set, 10).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 20).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 30).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	bool contains_10 = false;
	if (fun_set_int_contains(&set, 10, &contains_10).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(contains_10 == true)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	bool contains_99 = false;
	if (fun_set_int_contains(&set, 99, &contains_99).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(contains_99 == false)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_set_int_destroy(&set);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_set_int_add_contains");
}

void test_fun_set_int_no_duplicates(void)
{
	intSetResult create_result = fun_set_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intSet set = create_result.value;

	if (fun_set_int_add(&set, 42).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 42).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 42).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	// Size should still be 1 (no duplicates)
	if (!(fun_set_int_size(&set) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_set_int_destroy(&set);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_set_int_no_duplicates");
}

void test_fun_set_int_remove(void)
{
	intSetResult create_result = fun_set_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intSet set = create_result.value;

	if (fun_set_int_add(&set, 1).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 3).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(fun_set_int_size(&set) == 3)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_set_int_remove(&set, 2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_set_int_size(&set) == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	bool contains_2 = true;
	if (fun_set_int_contains(&set, 2, &contains_2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(contains_2 == false)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult remove_result = fun_set_int_remove(&set, 999);
	if (!(fun_error_is_error(remove_result))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_set_int_destroy(&set);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_set_int_remove");
}

void test_fun_set_int_size(void)
{
	intSetResult create_result = fun_set_int_create(16);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intSet set = create_result.value;

	if (!(fun_set_int_size(&set) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_set_int_add(&set, 1).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_set_int_size(&set) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_set_int_add(&set, 2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 3).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_set_int_add(&set, 4).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_set_int_size(&set) == 4)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_set_int_remove(&set, 2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_set_int_size(&set) == 3)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_set_int_destroy(&set);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_set_int_size");
}

void test_fun_set_int_many_elements(void)
{
	intSetResult create_result = fun_set_int_create(32);
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intSet set = create_result.value;

	// Add 100 elements
	for (int i = 0; i < 100; i++) {
		if (fun_set_int_add(&set, i).code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
	}

	if (!(fun_set_int_size(&set) == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Verify all elements present
	for (int i = 0; i < 100; i++) {
		bool contains = false;
		if (fun_set_int_contains(&set, i, &contains).code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
		if (!(contains == true)) {
			fun_console_write_line("FAIL: assertion");
			return;
		}
	}

	// Remove half
	for (int i = 0; i < 50; i++) {
		if (fun_set_int_remove(&set, i).code != 0) {
			fun_console_write_line("FAIL: ASSERT_ERROR_OK");
			return;
		}
	}

	if (!(fun_set_int_size(&set) == 50)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_set_int_destroy(&set);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_set_int_many_elements");
}

int main(void)
{
	fun_console_write_line("Running Set Module Unit Tests");
	fun_console_write_line("==============================");
	fun_console_write_line("");

	test_fun_set_int_add_contains();
	test_fun_set_int_no_duplicates();
	test_fun_set_int_remove();
	test_fun_set_int_size();
	test_fun_set_int_many_elements();

	fun_console_write_line("");
	fun_console_write_line("==============================");
	fun_console_write_line("All set tests completed");
	return 0;
}
