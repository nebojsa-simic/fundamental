#include "fundamental/rbtree/rbtree.h"
#include "fundamental/console/console.h"

DEFINE_RBTREE_TYPE(int, int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

void test_fun_rbtree_int_int_insert_get(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintRBTree tree = create_result.value;

	if (fun_rbtree_int_int_insert(&tree, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 2, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 3, 300).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(fun_rbtree_int_int_get(&tree, 1) == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 2) == 200)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 3) == 300)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_rbtree_int_int_insert_get");
}

void test_fun_rbtree_int_int_update(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintRBTree tree = create_result.value;

	if (fun_rbtree_int_int_insert(&tree, 5, 500).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 5) == 500)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Update existing key
	if (fun_rbtree_int_int_insert(&tree, 5, 999).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 5) == 999)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_size(&tree) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_rbtree_int_int_update");
}

void test_fun_rbtree_int_int_contains(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintRBTree tree = create_result.value;

	if (fun_rbtree_int_int_insert(&tree, 42, 4200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	bool contains_42 = false;
	if (fun_rbtree_int_int_contains(&tree, 42, &contains_42).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(contains_42 == true)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	bool contains_99 = false;
	if (fun_rbtree_int_int_contains(&tree, 99, &contains_99).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(contains_99 == false)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_rbtree_int_int_contains");
}

void test_fun_rbtree_int_int_remove(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintRBTree tree = create_result.value;

	if (fun_rbtree_int_int_insert(&tree, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 2, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 3, 300).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(fun_rbtree_int_int_size(&tree) == 3)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_rbtree_int_int_remove(&tree, 2).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_rbtree_int_int_size(&tree) == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (!(fun_rbtree_int_int_get(&tree, 1) == 100)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 3) == 300)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult remove_result = fun_rbtree_int_int_remove(&tree, 999);
	if (!(fun_error_is_error(remove_result))) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_rbtree_int_int_remove");
}

void test_fun_rbtree_int_int_size(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintRBTree tree = create_result.value;

	if (!(fun_rbtree_int_int_size(&tree) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_rbtree_int_int_insert(&tree, 1, 100).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_rbtree_int_int_size(&tree) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_rbtree_int_int_insert(&tree, 2, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_rbtree_int_int_size(&tree) == 2)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	if (fun_rbtree_int_int_remove(&tree, 1).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (!(fun_rbtree_int_int_size(&tree) == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_rbtree_int_int_size");
}

void test_fun_rbtree_int_int_ordered(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	if (create_result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_RESULT_OK");
		return;
	}

	intintRBTree tree = create_result.value;

	// Insert in random order
	if (fun_rbtree_int_int_insert(&tree, 50, 500).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 30, 300).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 70, 700).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 20, 200).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 40, 400).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 60, 600).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}
	if (fun_rbtree_int_int_insert(&tree, 80, 800).code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	if (!(fun_rbtree_int_int_size(&tree) == 7)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// All values retrievable
	if (!(fun_rbtree_int_int_get(&tree, 20) == 200)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 30) == 300)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 40) == 400)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 50) == 500)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 60) == 600)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 70) == 700)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(fun_rbtree_int_int_get(&tree, 80) == 800)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	if (destroy.code != 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR_OK");
		return;
	}

	print_test_result("fun_rbtree_int_int_ordered");
}

int main(void)
{
	fun_console_write_line("Running RBTree Module Unit Tests");
	fun_console_write_line("================================");
	fun_console_write_line("");

	test_fun_rbtree_int_int_insert_get();
	test_fun_rbtree_int_int_update();
	test_fun_rbtree_int_int_contains();
	test_fun_rbtree_int_int_remove();
	test_fun_rbtree_int_int_size();
	test_fun_rbtree_int_int_ordered();

	fun_console_write_line("");
	fun_console_write_line("================================");
	fun_console_write_line("All rbtree tests completed");
	return 0;
}
