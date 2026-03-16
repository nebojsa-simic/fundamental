#include "filesystem/filesystem.h"
#include "memory/memory.h"
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

static void test_path_join_base_relative(void)
{
	const char *test_name = "fun_path_join - base and relative";
	Path base, relative, output;
	const char *base_components[] = { "home", "user" };
	const char *rel_components[] = { "documents" };
	const char *out_components[MAX_COMPONENTS];

	base.components = base_components;
	base.count = 2;
	base.is_absolute = true;

	relative.components = rel_components;
	relative.count = 1;
	relative.is_absolute = false;

	output.components = out_components;

	ErrorResult result = fun_path_join(base, relative, &output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 3 &&
				  output.is_absolute == true);

	print_test_result(test_name, passed);
}

static void test_path_join_with_dot_dot(void)
{
	const char *test_name = "fun_path_join - with .. component";
	Path base, relative, output;
	const char *base_components[] = { "home", "user", "documents" };
	const char *rel_components[] = { "..", "pictures" };
	const char *out_components[MAX_COMPONENTS];

	base.components = base_components;
	base.count = 3;
	base.is_absolute = true;

	relative.components = rel_components;
	relative.count = 2;
	relative.is_absolute = false;

	output.components = out_components;

	ErrorResult result = fun_path_join(base, relative, &output);

	// Should remove "documents" and add "pictures"
	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 3);

	print_test_result(test_name, passed);
}

static void test_path_join_with_dot(void)
{
	const char *test_name = "fun_path_join - with . component";
	Path base, relative, output;
	const char *base_components[] = { "home", "user" };
	const char *rel_components[] = { ".", "documents" };
	const char *out_components[MAX_COMPONENTS];

	base.components = base_components;
	base.count = 2;
	base.is_absolute = true;

	relative.components = rel_components;
	relative.count = 2;
	relative.is_absolute = false;

	output.components = out_components;

	ErrorResult result = fun_path_join(base, relative, &output);

	// . should be ignored
	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 3);

	print_test_result(test_name, passed);
}

static void test_path_join_empty_relative(void)
{
	const char *test_name = "fun_path_join - empty relative";
	Path base, relative, output;
	const char *base_components[] = { "home", "user" };
	const char *out_components[MAX_COMPONENTS];

	base.components = base_components;
	base.count = 2;
	base.is_absolute = true;

	relative.components = NULL;
	relative.count = 0;
	relative.is_absolute = false;

	output.components = out_components;

	ErrorResult result = fun_path_join(base, relative, &output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 2);

	print_test_result(test_name, passed);
}

static void test_path_normalize_with_dot(void)
{
	const char *test_name = "fun_path_normalize - with . components";
	Path path, output;
	const char *path_components[] = { "home", ".", "user", ".", "documents" };
	const char *out_components[MAX_COMPONENTS];

	path.components = path_components;
	path.count = 5;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_normalize(path, &output);

	// All . should be removed
	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 3);

	print_test_result(test_name, passed);
}

static void test_path_normalize_with_dot_dot(void)
{
	const char *test_name = "fun_path_normalize - with .. components";
	Path path, output;
	const char *path_components[] = { "home", "user", "..", "documents" };
	const char *out_components[MAX_COMPONENTS];

	path.components = path_components;
	path.count = 4;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_normalize(path, &output);

	// user and .. should cancel out
	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 2);

	print_test_result(test_name, passed);
}

static void test_path_normalize_already_clean(void)
{
	const char *test_name = "fun_path_normalize - already clean";
	Path path, output;
	const char *path_components[] = { "home", "user", "documents" };
	const char *out_components[MAX_COMPONENTS];

	path.components = path_components;
	path.count = 3;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_normalize(path, &output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 3);

	print_test_result(test_name, passed);
}

static void test_path_get_parent_nested(void)
{
	const char *test_name = "fun_path_get_parent - nested path";
	Path path, output;
	const char *path_components[] = { "home", "user", "documents", "file.txt" };
	const char *out_components[MAX_COMPONENTS];

	path.components = path_components;
	path.count = 4;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_get_parent(path, &output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 3);

	print_test_result(test_name, passed);
}

static void test_path_get_parent_single(void)
{
	const char *test_name = "fun_path_get_parent - single component";
	Path path, output;
	const char *path_components[] = { "home" };
	const char *out_components[MAX_COMPONENTS];

	path.components = path_components;
	path.count = 1;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_get_parent(path, &output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 0);

	print_test_result(test_name, passed);
}

static void test_path_get_filename_nested(void)
{
	const char *test_name = "fun_path_get_filename - nested path";
	Path path, output;
	const char *path_components[] = { "home", "user", "documents", "file.txt" };
	const char *out_components[1];

	path.components = path_components;
	path.count = 4;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_get_filename(path, &output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 1);

	print_test_result(test_name, passed);
}

static void test_path_get_filename_single(void)
{
	const char *test_name = "fun_path_get_filename - single component";
	Path path, output;
	const char *path_components[] = { "file.txt" };
	const char *out_components[1];

	path.components = path_components;
	path.count = 1;
	path.is_absolute = false;

	output.components = out_components;

	ErrorResult result = fun_path_get_filename(path, &output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output.count == 1);

	print_test_result(test_name, passed);
}

int main(void)
{
	printf("=== Path Operations Tests ===\n\n");

	// Join tests
	test_path_join_base_relative();
	test_path_join_with_dot_dot();
	test_path_join_with_dot();
	test_path_join_empty_relative();

	// Normalize tests
	test_path_normalize_with_dot();
	test_path_normalize_with_dot_dot();
	test_path_normalize_already_clean();

	// Parent/Filename tests
	test_path_get_parent_nested();
	test_path_get_parent_single();
	test_path_get_filename_nested();
	test_path_get_filename_single();

	printf("\n=== Results ===\n");
	printf("Passed: %d\n", tests_passed);
	printf("Failed: %d\n", tests_failed);

	return tests_failed > 0 ? 1 : 0;
}
