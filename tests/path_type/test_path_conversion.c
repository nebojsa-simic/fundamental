#include "fundamental/filesystem/filesystem.h"
#include "fundamental/memory/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m\033[0m"

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

static void test_path_from_string_absolute_unix(void)
{
	const char *test_name = "fun_path_from_string - absolute Unix path";
	Path path;
	const char *components[MAX_COMPONENTS];
	path.components = components;

	ErrorResult result =
		fun_path_from_string("/home/user/documents/file.txt", &path);

	int passed = (result.code == ERROR_CODE_NO_ERROR &&
				  path.is_absolute == true && path.count == 4);

	print_test_result(test_name, passed);
}

static void test_path_from_string_relative(void)
{
	const char *test_name = "fun_path_from_string - relative path";
	Path path;
	const char *components[MAX_COMPONENTS];
	path.components = components;

	ErrorResult result = fun_path_from_string("documents/file.txt", &path);

	int passed = (result.code == ERROR_CODE_NO_ERROR &&
				  path.is_absolute == false && path.count == 2);

	print_test_result(test_name, passed);
}

static void test_path_from_string_with_dot_components(void)
{
	const char *test_name = "fun_path_from_string - path with . and ..";
	Path path;
	const char *components[MAX_COMPONENTS];
	path.components = components;

	ErrorResult result = fun_path_from_string("./documents/../file.txt", &path);

	// Should preserve . and .. without normalization
	int passed = (result.code == ERROR_CODE_NO_ERROR && path.count == 4);

	print_test_result(test_name, passed);
}

static void test_path_from_string_multiple_separators(void)
{
	const char *test_name = "fun_path_from_string - multiple separators";
	Path path;
	const char *components[MAX_COMPONENTS];
	path.components = components;

	ErrorResult result = fun_path_from_string("/home//user///documents", &path);

	// Multiple separators should be treated as single separator
	int passed = (result.code == ERROR_CODE_NO_ERROR && path.count == 3);

	print_test_result(test_name, passed);
}

static void test_path_from_string_trailing_separator(void)
{
	const char *test_name = "fun_path_from_string - trailing separator";
	Path path;
	const char *components[MAX_COMPONENTS];
	path.components = components;

	ErrorResult result = fun_path_from_string("/home/user/", &path);

	int passed = (result.code == ERROR_CODE_NO_ERROR && path.count == 2);

	print_test_result(test_name, passed);
}

static void test_path_from_string_null_input(void)
{
	const char *test_name = "fun_path_from_string - NULL input";
	Path path;
	const char *components[MAX_COMPONENTS];
	path.components = components;

	ErrorResult result = fun_path_from_string(NULL, &path);

	int passed = (result.code == ERROR_CODE_NULL_POINTER);

	print_test_result(test_name, passed);
}

static void test_path_from_string_empty_string(void)
{
	const char *test_name = "fun_path_from_string - empty string";
	Path path;
	const char *components[MAX_COMPONENTS];
	path.components = components;

	ErrorResult result = fun_path_from_string("", &path);

	int passed = (result.code == ERROR_CODE_NO_ERROR && path.count == 0 &&
				  path.is_absolute == false);

	print_test_result(test_name, passed);
}

static void test_path_to_string_absolute(void)
{
	const char *test_name = "fun_path_to_string - absolute path";
	Path path;
	const char *path_components[] = { "home", "user", "documents" };
	path.components = path_components;
	path.count = 3;
	path.is_absolute = true;

	char buffer[512];
	ErrorResult result = fun_path_to_string(path, buffer, sizeof(buffer));

	// Should start with separator
	int passed = (result.code == ERROR_CODE_NO_ERROR &&
				  (buffer[0] == '/' || buffer[0] == '\\'));

	print_test_result(test_name, passed);
}

static void test_path_to_string_relative(void)
{
	const char *test_name = "fun_path_to_string - relative path";
	Path path;
	const char *path_components[] = { "documents", "file.txt" };
	path.components = path_components;
	path.count = 2;
	path.is_absolute = false;

	char buffer[512];
	ErrorResult result = fun_path_to_string(path, buffer, sizeof(buffer));

	// Should NOT start with separator
	int passed = (result.code == ERROR_CODE_NO_ERROR && buffer[0] != '/' &&
				  buffer[0] != '\\');

	print_test_result(test_name, passed);
}

static void test_path_to_string_empty_path(void)
{
	const char *test_name = "fun_path_to_string - empty path";
	Path path;
	path.components = NULL;
	path.count = 0;
	path.is_absolute = false;

	char buffer[512];
	ErrorResult result = fun_path_to_string(path, buffer, sizeof(buffer));

	int passed =
		(result.code == ERROR_CODE_NO_ERROR && strcmp(buffer, ".") == 0);

	print_test_result(test_name, passed);
}

static void test_path_to_string_buffer_too_small(void)
{
	const char *test_name = "fun_path_to_string - buffer too small";
	Path path;
	const char *path_components[] = { "verylongcomponentname" };
	path.components = path_components;
	path.count = 1;
	path.is_absolute = false;

	char buffer[5];
	ErrorResult result = fun_path_to_string(path, buffer, sizeof(buffer));

	int passed = (result.code == ERROR_CODE_BUFFER_TOO_SMALL);

	print_test_result(test_name, passed);
}

int main(void)
{
	printf("=== Path Conversion Tests ===\n\n");

	// String to Path tests
	test_path_from_string_absolute_unix();
	test_path_from_string_relative();
	test_path_from_string_with_dot_components();
	test_path_from_string_multiple_separators();
	test_path_from_string_trailing_separator();
	test_path_from_string_null_input();
	test_path_from_string_empty_string();

	// Path to String tests
	test_path_to_string_absolute();
	test_path_to_string_relative();
	test_path_to_string_empty_path();
	test_path_to_string_buffer_too_small();

	printf("\n=== Results ===\n");
	printf("Passed: %d\n", tests_passed);
	printf("Failed: %d\n", tests_failed);

	return tests_failed > 0 ? 1 : 0;
}
