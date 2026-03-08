#include "filesystem/filesystem.h"
#include "memory/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

// Helper function to check if directory exists using stdlib
static int directory_exists(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0) {
		return 0;
	}
	return S_ISDIR(st.st_mode);
}

// Helper function to remove directory
static void remove_dir(const char *path)
{
	rmdir(path);
}

// Helper function to print test result
static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, test_name);
	} else {
		printf("%s %s\n", RED_CROSS, test_name);
	}
}

// Test: fun_filesystem_create_directory
static void test_fun_filesystem_create_directory(void)
{
	const char *test_path = "test_output/basic_dir";

	// Remove if exists from previous run
	remove_dir(test_path);

	// Create directory
	ErrorResult result = fun_filesystem_create_directory(test_path);

	// Verify using stdlib stat
	if (result.code == ERROR_CODE_NO_ERROR && directory_exists(test_path)) {
		remove_dir(test_path);
		print_test_result("fun_filesystem_create_directory", 1);
	} else {
		print_test_result("fun_filesystem_create_directory", 0);
	}
}

// Test: fun_filesystem_create_directory_nested
static void test_fun_filesystem_create_directory_nested(void)
{
	const char *test_path = "test_output/nested/level1/level2";

	// Clean up from previous run
	remove_dir("test_output/nested/level1/level2");
	remove_dir("test_output/nested/level1");
	remove_dir("test_output/nested");

	// Create nested directories
	ErrorResult result = fun_filesystem_create_directory(test_path);

	// Verify all levels exist
	int exists = directory_exists("test_output/nested") &&
				 directory_exists("test_output/nested/level1") &&
				 directory_exists("test_output/nested/level1/level2");

	if (result.code == ERROR_CODE_NO_ERROR && exists) {
		remove_dir("test_output/nested/level1/level2");
		remove_dir("test_output/nested/level1");
		remove_dir("test_output/nested");
		print_test_result("fun_filesystem_create_directory_nested", 1);
	} else {
		print_test_result("fun_filesystem_create_directory_nested", 0);
	}
}

// Test: fun_filesystem_create_directory_idempotent
static void test_fun_filesystem_create_directory_idempotent(void)
{
	const char *test_path = "test_output/existing_dir";

	// Create directory first
	fun_filesystem_create_directory(test_path);

	// Try to create again - should succeed (idempotent)
	ErrorResult result = fun_filesystem_create_directory(test_path);

	if (result.code == ERROR_CODE_NO_ERROR && directory_exists(test_path)) {
		remove_dir(test_path);
		print_test_result("fun_filesystem_create_directory_idempotent", 1);
	} else {
		print_test_result("fun_filesystem_create_directory_idempotent", 0);
	}
}

// Test: fun_filesystem_create_directory_invalid
static void test_fun_filesystem_create_directory_invalid(void)
{
	int passed = 1;

	// NULL path should fail
	ErrorResult result = fun_filesystem_create_directory(NULL);
	if (result.code == ERROR_CODE_NO_ERROR) {
		passed = 0;
	}

	// Empty path should fail
	result = fun_filesystem_create_directory("");
	if (result.code == ERROR_CODE_NO_ERROR) {
		passed = 0;
	}

	print_test_result("fun_filesystem_create_directory_invalid", passed);
}

// Test: fun_filesystem_remove_directory
static void test_fun_filesystem_remove_directory(void)
{
	const char *test_path = "test_output/to_remove";

	// Create directory first
	fun_filesystem_create_directory(test_path);

	// Verify it exists
	if (!directory_exists(test_path)) {
		print_test_result("fun_filesystem_remove_directory", 0);
		return;
	}

	// Remove it
	ErrorResult result = fun_filesystem_remove_directory(test_path);

	// Verify it's gone
	int exists_after = directory_exists(test_path);

	if (result.code == ERROR_CODE_NO_ERROR && !exists_after) {
		print_test_result("fun_filesystem_remove_directory", 1);
	} else {
		print_test_result("fun_filesystem_remove_directory", 0);
	}
}

// Test: fun_filesystem_remove_directory_not_found
static void test_fun_filesystem_remove_directory_not_found(void)
{
	ErrorResult result =
		fun_filesystem_remove_directory("test_output/does_not_exist");

	int passed = (result.code == ERROR_CODE_DIRECTORY_NOT_FOUND);
	print_test_result("fun_filesystem_remove_directory_not_found", passed);
}

// Test: fun_filesystem_list_directory
static void test_fun_filesystem_list_directory(void)
{
	const char *test_path = "test_output/list_test";

	// Create test directory
	fun_filesystem_create_directory(test_path);

	if (!directory_exists(test_path)) {
		print_test_result("fun_filesystem_list_directory", 0);
		return;
	}

	// Allocate buffer for listing
	MemoryResult alloc_result = fun_memory_allocate(4096);
	if (fun_error_is_error(alloc_result.error)) {
		remove_dir(test_path);
		print_test_result("fun_filesystem_list_directory", 0);
		return;
	}

	Memory buffer = alloc_result.value;
	ErrorResult result = fun_filesystem_list_directory(test_path, buffer);

	// Empty directory should return empty buffer
	int buffer_empty = ((char *)buffer)[0] == '\0';

	fun_memory_free(&buffer);
	remove_dir(test_path);

	if (result.code == ERROR_CODE_NO_ERROR && buffer_empty) {
		print_test_result("fun_filesystem_list_directory", 1);
	} else {
		print_test_result("fun_filesystem_list_directory", 0);
	}
}

// Test: fun_path_join
static void test_fun_path_join(void)
{
	char output[512];

	ErrorResult result = fun_path_join("/home/user", "documents", output);

	int passed =
		(result.code == ERROR_CODE_NO_ERROR && output[0] != '\0' &&
		 (strstr(output, "home") != NULL || strstr(output, "user") != NULL));

	print_test_result("fun_path_join", passed);
}

// Test: fun_path_normalize
static void test_fun_path_normalize(void)
{
	char output[512];

	ErrorResult result = fun_path_normalize("/home/./user/./docs", output);

	// Should not contain /./
	int has_dot_component = (strstr(output, "/./") != NULL);

	int passed = (result.code == ERROR_CODE_NO_ERROR && !has_dot_component);

	print_test_result("fun_path_normalize", passed);
}

// Test: fun_path_get_parent
static void test_fun_path_get_parent(void)
{
	char output[512];

	ErrorResult result =
		fun_path_get_parent("/home/user/documents/file.txt", output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output[0] != '\0' &&
				  strstr(output, "documents") != NULL);

	print_test_result("fun_path_get_parent", passed);
}

// Test: fun_path_get_filename
static void test_fun_path_get_filename(void)
{
	char output[512];

	ErrorResult result =
		fun_path_get_filename("/home/user/documents/file.txt", output);

	int passed = (result.code == ERROR_CODE_NO_ERROR && output[0] != '\0' &&
				  strcmp(output, "file.txt") == 0);

	print_test_result("fun_path_get_filename", passed);
}

// Test: fun_path_separator
static void test_fun_path_separator(void)
{
	char sep = fun_path_separator();

#ifdef _WIN32
	int passed = (sep == '\\');
#else
	int passed = (sep == '/');
#endif

	print_test_result("fun_path_separator", passed);
}

// Test: fun_filesystem_null_parameters
static void test_fun_filesystem_null_parameters(void)
{
	int passed = 1;

	// Test NULL path for create
	ErrorResult result = fun_filesystem_create_directory(NULL);
	if (result.code == ERROR_CODE_NO_ERROR) {
		passed = 0;
	}

	// Test NULL output for path operations
	result = fun_path_join("/base", "/relative", NULL);
	if (result.code == ERROR_CODE_NO_ERROR) {
		passed = 0;
	}

	result = fun_path_normalize("/path", NULL);
	if (result.code == ERROR_CODE_NO_ERROR) {
		passed = 0;
	}

	result = fun_path_get_parent("/path", NULL);
	if (result.code == ERROR_CODE_NO_ERROR) {
		passed = 0;
	}

	result = fun_path_get_filename("/path", NULL);
	if (result.code == ERROR_CODE_NO_ERROR) {
		passed = 0;
	}

	print_test_result("fun_filesystem_null_parameters", passed);
}

// Setup and cleanup
static void setup_tests(void)
{
	fun_filesystem_create_directory("test_output");
}

static void cleanup_tests(void)
{
	remove_dir("test_output");
}

int main(void)
{
	printf("Running filesystem module tests:\n");

	setup_tests();

	test_fun_path_separator();
	test_fun_filesystem_create_directory();
	test_fun_filesystem_create_directory_nested();
	test_fun_filesystem_create_directory_idempotent();
	test_fun_filesystem_create_directory_invalid();
	test_fun_filesystem_remove_directory();
	test_fun_filesystem_remove_directory_not_found();
	test_fun_filesystem_list_directory();
	test_fun_path_join();
	test_fun_path_normalize();
	test_fun_path_get_parent();
	test_fun_path_get_filename();
	test_fun_filesystem_null_parameters();

	cleanup_tests();

	return 0;
}
