#include "fundamental/filesystem/filesystem.h"
#include "fundamental/memory/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

#define MAX_COMPONENTS 32

static int tests_passed = 0;
static int tests_failed = 0;

static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, test_name);
		tests_passed++;
	} else {
		printf("%s %s\n", RED_CROSS, test_name);
		tests_failed++;
	}
}

static void test_path_get_component_valid_index(void)
{
	const char *test_name = "fun_path_get_component - valid index";
	Path path;
	const char *path_components[] = { "home", "user", "documents" };

	path.components = path_components;
	path.count = 3;
	path.is_absolute = true;

	String component = fun_path_get_component(path, 1);

	int passed = (component != NULL && strcmp(component, "user") == 0);

	print_test_result(test_name, passed);
}

static void test_path_get_component_invalid_index(void)
{
	const char *test_name = "fun_path_get_component - invalid index";
	Path path;
	const char *path_components[] = { "home", "user" };

	path.components = path_components;
	path.count = 2;
	path.is_absolute = true;

	String component = fun_path_get_component(path, 5);

	int passed = (component == NULL);

	print_test_result(test_name, passed);
}

static void test_path_get_component_zero_index(void)
{
	const char *test_name = "fun_path_get_component - index 0";
	Path path;
	const char *path_components[] = { "home", "user", "documents" };

	path.components = path_components;
	path.count = 3;
	path.is_absolute = true;

	String component = fun_path_get_component(path, 0);

	int passed = (component != NULL && strcmp(component, "home") == 0);

	print_test_result(test_name, passed);
}

static void test_path_component_count(void)
{
	const char *test_name = "fun_path_component_count";
	Path path;
	const char *path_components[] = { "home", "user", "documents", "file.txt" };

	path.components = path_components;
	path.count = 4;
	path.is_absolute = true;

	size_t count = fun_path_component_count(path);

	int passed = (count == 4);

	print_test_result(test_name, passed);
}

static void test_path_component_count_empty(void)
{
	const char *test_name = "fun_path_component_count - empty path";
	Path path;

	path.components = NULL;
	path.count = 0;
	path.is_absolute = false;

	size_t count = fun_path_component_count(path);

	int passed = (count == 0);

	print_test_result(test_name, passed);
}

static void test_path_is_valid_valid_path(void)
{
	const char *test_name = "fun_path_is_valid - valid path";
	Path path;
	const char *path_components[] = { "home", "user", "documents" };

	path.components = path_components;
	path.count = 3;
	path.is_absolute = true;

	int result = fun_path_is_valid(path);

	int passed = (result == 1);

	print_test_result(test_name, passed);
}

static void test_path_is_valid_empty_path(void)
{
	const char *test_name = "fun_path_is_valid - empty path";
	Path path;

	path.components = NULL;
	path.count = 0;
	path.is_absolute = false;

	int result = fun_path_is_valid(path);

	int passed = (result == 0);

	print_test_result(test_name, passed);
}

static void test_path_get_component_last(void)
{
	const char *test_name = "fun_path_get_component - last component";
	Path path;
	const char *path_components[] = { "home", "user", "documents" };

	path.components = path_components;
	path.count = 3;
	path.is_absolute = true;

	String component = fun_path_get_component(path, 2);

	int passed = (component != NULL && strcmp(component, "documents") == 0);

	print_test_result(test_name, passed);
}

static void test_path_get_component_out_of_bounds_zero_count(void)
{
	const char *test_name = "fun_path_get_component - out of bounds (count=0)";
	Path path;

	path.components = NULL;
	path.count = 0;
	path.is_absolute = false;

	String component = fun_path_get_component(path, 0);

	int passed = (component == NULL);

	print_test_result(test_name, passed);
}

int main(void)
{
	printf("=== Path Component Access Tests ===\n\n");

	// Component access tests
	test_path_get_component_valid_index();
	test_path_get_component_invalid_index();
	test_path_get_component_zero_index();
	test_path_get_component_last();
	test_path_get_component_out_of_bounds_zero_count();

	// Component count tests
	test_path_component_count();
	test_path_component_count_empty();

	// Validation tests
	test_path_is_valid_valid_path();
	test_path_is_valid_empty_path();

	printf("\n=== Results ===\n");
	printf("Passed: %d\n", tests_passed);
	printf("Failed: %d\n", tests_failed);

	return tests_failed > 0 ? 1 : 0;
}
