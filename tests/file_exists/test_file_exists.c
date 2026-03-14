#include "filesystem/filesystem.h"
#include "memory/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

// Helper function to check if file exists using stdlib
static int file_exists_stdlib(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0) {
		return 0;
	}
	return !S_ISDIR(st.st_mode);
}

// Helper function to check if directory exists using stdlib
static int directory_exists_stdlib(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0) {
		return 0;
	}
	return S_ISDIR(st.st_mode);
}

// Helper function to create a test file
static void create_test_file(const char *path)
{
	FILE *f = fopen(path, "w");
	if (f) {
		fprintf(f, "test content\n");
		fclose(f);
	}
}

// Helper function to remove a file
static void remove_test_file(const char *path)
{
	remove(path);
}

// Helper function to remove directory
static void remove_test_dir(const char *path)
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

// ============================================================================
// Tests for fun_file_exists
// ============================================================================

static void test_fun_file_exists_existing_file(void)
{
	const char *test_path = "test_output/file_exists_test.txt";
	int passed = 0;

	// Clean up from previous run
	remove_test_file(test_path);

	// Create test file
	create_test_file(test_path);

	// Test file exists
	boolResult result = fun_file_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	// Cleanup
	remove_test_file(test_path);

	print_test_result("fun_file_exists: existing file", passed);
}

static void test_fun_file_exists_nonexistent_file(void)
{
	const char *test_path = "test_output/nonexistent_file.txt";
	int passed = 0;

	// Ensure file doesn't exist
	remove_test_file(test_path);

	// Test file doesn't exist
	boolResult result = fun_file_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_file_exists: nonexistent file", passed);
}

static void test_fun_file_exists_directory_path(void)
{
	const char *test_path = "test_output/existing_dir";
	int passed = 0;

	// Clean up and create test directory
	remove_test_dir(test_path);

#ifdef _WIN32
	_mkdir(test_path);
#else
	mkdir(test_path, 0755);
#endif

	// Test that file_exists returns false for directory
	boolResult result = fun_file_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	// Cleanup
	remove_test_dir(test_path);

	print_test_result("fun_file_exists: directory path", passed);
}

static void test_fun_file_exists_invalid_path(void)
{
	int passed = 0;

	// Test NULL path
	boolResult result = fun_file_exists(NULL);

	if (fun_error_is_error(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_file_exists: NULL path", passed);
}

// ============================================================================
// Tests for fun_directory_exists
// ============================================================================

static void test_fun_directory_exists_existing_directory(void)
{
	const char *test_path = "test_output/dir_exists_test";
	int passed = 0;

	// Clean up and create test directory
	remove_test_dir(test_path);

#ifdef _WIN32
	_mkdir(test_path);
#else
	mkdir(test_path, 0755);
#endif

	// Test directory exists
	boolResult result = fun_directory_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	// Cleanup
	remove_test_dir(test_path);

	print_test_result("fun_directory_exists: existing directory", passed);
}

static void test_fun_directory_exists_nonexistent_directory(void)
{
	const char *test_path = "test_output/nonexistent_dir";
	int passed = 0;

	// Ensure directory doesn't exist
	remove_test_dir(test_path);

	// Test directory doesn't exist
	boolResult result = fun_directory_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_directory_exists: nonexistent directory", passed);
}

static void test_fun_directory_exists_file_path(void)
{
	const char *test_path = "test_output/existing_file.txt";
	int passed = 0;

	// Clean up and create test file
	remove_test_file(test_path);
	create_test_file(test_path);

	// Test that directory_exists returns false for file
	boolResult result = fun_directory_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	// Cleanup
	remove_test_file(test_path);

	print_test_result("fun_directory_exists: file path", passed);
}

static void test_fun_directory_exists_invalid_path(void)
{
	int passed = 0;

	// Test NULL path
	boolResult result = fun_directory_exists(NULL);

	if (fun_error_is_error(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_directory_exists: NULL path", passed);
}

// ============================================================================
// Tests for fun_path_exists
// ============================================================================

static void test_fun_path_exists_existing_file(void)
{
	const char *test_path = "test_output/path_exists_file.txt";
	int passed = 0;

	// Clean up and create test file
	remove_test_file(test_path);
	create_test_file(test_path);

	// Test path exists (as file)
	boolResult result = fun_path_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	// Cleanup
	remove_test_file(test_path);

	print_test_result("fun_path_exists: existing file", passed);
}

static void test_fun_path_exists_existing_directory(void)
{
	const char *test_path = "test_output/path_exists_dir";
	int passed = 0;

	// Clean up and create test directory
	remove_test_dir(test_path);

#ifdef _WIN32
	_mkdir(test_path);
#else
	mkdir(test_path, 0755);
#endif

	// Test path exists (as directory)
	boolResult result = fun_path_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	// Cleanup
	remove_test_dir(test_path);

	print_test_result("fun_path_exists: existing directory", passed);
}

static void test_fun_path_exists_nonexistent_path(void)
{
	const char *test_path = "test_output/nonexistent_path";
	int passed = 0;

	// Ensure path doesn't exist
	remove_test_file(test_path);
	remove_test_dir(test_path);

	// Test path doesn't exist
	boolResult result = fun_path_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_path_exists: nonexistent path", passed);
}

static void test_fun_path_exists_invalid_path(void)
{
	int passed = 0;

	// Test NULL path
	boolResult result = fun_path_exists(NULL);

	if (fun_error_is_error(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_path_exists: NULL path", passed);
}

// ============================================================================
// Main
// ============================================================================

int main(void)
{
	// Create test output directory
#ifdef _WIN32
	_mkdir("test_output");
#else
	mkdir("test_output", 0755);
#endif

	printf("=== File Existence Tests ===\n\n");

	printf("-- fun_file_exists tests --\n");
	test_fun_file_exists_existing_file();
	test_fun_file_exists_nonexistent_file();
	test_fun_file_exists_directory_path();
	test_fun_file_exists_invalid_path();

	printf("\n-- fun_directory_exists tests --\n");
	test_fun_directory_exists_existing_directory();
	test_fun_directory_exists_nonexistent_directory();
	test_fun_directory_exists_file_path();
	test_fun_directory_exists_invalid_path();

	printf("\n-- fun_path_exists tests --\n");
	test_fun_path_exists_existing_file();
	test_fun_path_exists_existing_directory();
	test_fun_path_exists_nonexistent_path();
	test_fun_path_exists_invalid_path();

	// Cleanup test output directory
	rmdir("test_output");

	printf("\n=== Tests Complete ===\n");

	return 0;
}
