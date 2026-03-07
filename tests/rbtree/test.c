#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../include/rbtree/rbtree.h"

DEFINE_RBTREE_TYPE(int, int)

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define ASSERT_ERROR_OK(result) assert((result).code == 0)
#define ASSERT_RESULT_OK(result) assert((result).error.code == 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_fun_rbtree_int_int_insert_get(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	ASSERT_RESULT_OK(create_result);

	intintRBTree tree = create_result.value;

	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 1, 100));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 2, 200));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 3, 300));

	assert(fun_rbtree_int_int_get(&tree, 1) == 100);
	assert(fun_rbtree_int_int_get(&tree, 2) == 200);
	assert(fun_rbtree_int_int_get(&tree, 3) == 300);

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_rbtree_int_int_insert_get");
}

void test_fun_rbtree_int_int_update(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	ASSERT_RESULT_OK(create_result);

	intintRBTree tree = create_result.value;

	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 5, 500));
	assert(fun_rbtree_int_int_get(&tree, 5) == 500);

	// Update existing key
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 5, 999));
	assert(fun_rbtree_int_int_get(&tree, 5) == 999);
	assert(fun_rbtree_int_int_size(&tree) == 1);

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_rbtree_int_int_update");
}

void test_fun_rbtree_int_int_contains(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	ASSERT_RESULT_OK(create_result);

	intintRBTree tree = create_result.value;

	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 42, 4200));

	bool contains_42 = false;
	ASSERT_ERROR_OK(fun_rbtree_int_int_contains(&tree, 42, &contains_42));
	assert(contains_42 == true);

	bool contains_99 = false;
	ASSERT_ERROR_OK(fun_rbtree_int_int_contains(&tree, 99, &contains_99));
	assert(contains_99 == false);

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_rbtree_int_int_contains");
}

void test_fun_rbtree_int_int_remove(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	ASSERT_RESULT_OK(create_result);

	intintRBTree tree = create_result.value;

	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 1, 100));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 2, 200));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 3, 300));

	assert(fun_rbtree_int_int_size(&tree) == 3);

	ASSERT_ERROR_OK(fun_rbtree_int_int_remove(&tree, 2));
	assert(fun_rbtree_int_int_size(&tree) == 2);

	assert(fun_rbtree_int_int_get(&tree, 1) == 100);
	assert(fun_rbtree_int_int_get(&tree, 3) == 300);

	ErrorResult remove_result = fun_rbtree_int_int_remove(&tree, 999);
	assert(fun_error_is_error(remove_result));

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_rbtree_int_int_remove");
}

void test_fun_rbtree_int_int_size(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	ASSERT_RESULT_OK(create_result);

	intintRBTree tree = create_result.value;

	assert(fun_rbtree_int_int_size(&tree) == 0);

	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 1, 100));
	assert(fun_rbtree_int_int_size(&tree) == 1);

	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 2, 200));
	assert(fun_rbtree_int_int_size(&tree) == 2);

	ASSERT_ERROR_OK(fun_rbtree_int_int_remove(&tree, 1));
	assert(fun_rbtree_int_int_size(&tree) == 1);

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_rbtree_int_int_size");
}

void test_fun_rbtree_int_int_ordered(void)
{
	intintRBTreeResult create_result = fun_rbtree_int_int_create();
	ASSERT_RESULT_OK(create_result);

	intintRBTree tree = create_result.value;

	// Insert in random order
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 50, 500));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 30, 300));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 70, 700));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 20, 200));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 40, 400));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 60, 600));
	ASSERT_ERROR_OK(fun_rbtree_int_int_insert(&tree, 80, 800));

	assert(fun_rbtree_int_int_size(&tree) == 7);

	// All values retrievable
	assert(fun_rbtree_int_int_get(&tree, 20) == 200);
	assert(fun_rbtree_int_int_get(&tree, 30) == 300);
	assert(fun_rbtree_int_int_get(&tree, 40) == 400);
	assert(fun_rbtree_int_int_get(&tree, 50) == 500);
	assert(fun_rbtree_int_int_get(&tree, 60) == 600);
	assert(fun_rbtree_int_int_get(&tree, 70) == 700);
	assert(fun_rbtree_int_int_get(&tree, 80) == 800);

	ErrorResult destroy = fun_rbtree_int_int_destroy(&tree);
	ASSERT_ERROR_OK(destroy);

	print_test_result("fun_rbtree_int_int_ordered");
}

int main(void)
{
	printf("Running RBTree Module Unit Tests\n");
	printf("================================\n\n");

	test_fun_rbtree_int_int_insert_get();
	test_fun_rbtree_int_int_update();
	test_fun_rbtree_int_int_contains();
	test_fun_rbtree_int_int_remove();
	test_fun_rbtree_int_int_size();
	test_fun_rbtree_int_int_ordered();

	printf("\n================================\n");
	printf("All rbtree tests completed\n");
	return 0;
}
