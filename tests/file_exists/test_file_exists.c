#include "fundamental/filesystem/filesystem.h"
#include "fundamental/memory/memory.h"
#include "fundamental/console/console.h"

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

#define MAX_TEST_COMPONENTS 16

// Helper: build a Path from a const char* string using caller-provided scratch space
static Path make_path(const char *str, char *buf, size_t buf_size,
					  const char **comps)
{
	Path p;
	p.components = comps;
	p.count = 0;
	p.is_absolute = false;
	if (str == NULL || buf == NULL)
		return p;
	size_t len = 0;
	while (str[len] && len < buf_size - 1) {
		buf[len] = str[len];
		len++;
	}
	buf[len] = '\0';
	fun_path_from_string(buf, &p);
	return p;
}

// Convenience macro: declare a Path named 'pvar' from string literal 'str'
#define MAKE_PATH(pvar, str)                        \
	char pvar##_buf_[512];                          \
	const char *pvar##_comps_[MAX_TEST_COMPONENTS]; \
	Path pvar = make_path(str, pvar##_buf_, sizeof(pvar##_buf_), pvar##_comps_)

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

// Suppress unused function warnings
static void suppress_unused(void)
{
	(void)file_exists_stdlib;
	(void)directory_exists_stdlib;
}

// Helper function to print test result
static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		fun_console_write(GREEN_CHECK);
		fun_console_write(" ");
		fun_console_write_line(test_name);
	} else {
		fun_console_write(RED_CROSS);
		fun_console_write(" ");
		fun_console_write_line(test_name);
	}
}

// ============================================================================
// Tests for fun_file_exists
// ============================================================================

static void test_fun_file_exists_existing_file(void)
{
	int passed = 0;

	// Test file exists
	MAKE_PATH(test_path, "test_output/file_exists_test.txt");
	boolResult result = fun_file_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	print_test_result("fun_file_exists: existing file", passed);
}

static void test_fun_file_exists_nonexistent_file(void)
{
	int passed = 0;

	// Test file doesn't exist
	MAKE_PATH(test_path, "test_output/nonexistent_file.txt");
	boolResult result = fun_file_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_file_exists: nonexistent file", passed);
}

static void test_fun_file_exists_directory_path(void)
{
	int passed = 0;

	// Test that file_exists returns false for directory
	MAKE_PATH(test_path, "test_output/existing_dir");
	boolResult result = fun_file_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_file_exists: directory path", passed);
}

static void test_fun_file_exists_invalid_path(void)
{
	int passed = 0;

	// NULL components with count>0 causes fun_path_to_string to return error
	Path null_path = { NULL, 1, false };
	boolResult result = fun_file_exists(null_path);

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
	int passed = 0;

	// Test directory exists
	MAKE_PATH(test_path, "test_output/dir_exists_test");
	boolResult result = fun_directory_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	print_test_result("fun_directory_exists: existing directory", passed);
}

static void test_fun_directory_exists_nonexistent_directory(void)
{
	int passed = 0;

	// Test directory doesn't exist
	MAKE_PATH(test_path, "test_output/nonexistent_dir");
	boolResult result = fun_directory_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_directory_exists: nonexistent directory", passed);
}

static void test_fun_directory_exists_file_path(void)
{
	int passed = 0;

	// Test that directory_exists returns false for file
	MAKE_PATH(test_path, "test_output/existing_file.txt");
	boolResult result = fun_directory_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_directory_exists: file path", passed);
}

static void test_fun_directory_exists_invalid_path(void)
{
	int passed = 0;

	// NULL components with count>0 causes fun_path_to_string to return error
	Path null_path = { NULL, 1, false };
	boolResult result = fun_directory_exists(null_path);

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
	int passed = 0;

	// Test path exists (as file)
	MAKE_PATH(test_path, "test_output/path_exists_file.txt");
	boolResult result = fun_path_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	print_test_result("fun_path_exists: existing file", passed);
}

static void test_fun_path_exists_existing_directory(void)
{
	int passed = 0;

	// Test path exists (as directory)
	MAKE_PATH(test_path, "test_output/path_exists_dir");
	boolResult result = fun_path_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == true) {
		passed = 1;
	}

	print_test_result("fun_path_exists: existing directory", passed);
}

static void test_fun_path_exists_nonexistent_path(void)
{
	int passed = 0;

	// Test path doesn't exist
	MAKE_PATH(test_path, "test_output/nonexistent_path");
	boolResult result = fun_path_exists(test_path);

	if (fun_error_is_ok(result.error) && result.value == false) {
		passed = 1;
	}

	print_test_result("fun_path_exists: nonexistent path", passed);
}

static void test_fun_path_exists_invalid_path(void)
{
	int passed = 0;

	// NULL components with count>0 causes fun_path_to_string to return error
	Path null_path = { NULL, 1, false };
	boolResult result = fun_path_exists(null_path);

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
	suppress_unused();

	fun_console_write_line("=== File Existence Tests ===");
	fun_console_write_line("");

	fun_console_write_line("-- fun_file_exists tests --");
	test_fun_file_exists_existing_file();
	test_fun_file_exists_nonexistent_file();
	test_fun_file_exists_directory_path();
	test_fun_file_exists_invalid_path();

	fun_console_write_line("");
	fun_console_write_line("-- fun_directory_exists tests --");
	test_fun_directory_exists_existing_directory();
	test_fun_directory_exists_nonexistent_directory();
	test_fun_directory_exists_file_path();
	test_fun_directory_exists_invalid_path();

	fun_console_write_line("");
	fun_console_write_line("-- fun_path_exists tests --");
	test_fun_path_exists_existing_file();
	test_fun_path_exists_existing_directory();
	test_fun_path_exists_nonexistent_path();
	test_fun_path_exists_invalid_path();

	fun_console_write_line("");
	fun_console_write_line("=== Tests Complete ===");

	return 0;
}
