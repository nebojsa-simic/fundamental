#include "fundamental/filesystem/filesystem.h"
#include "fundamental/memory/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
#define MAX_PATH_COMPONENTS 32

// Static work memory for walk tests (40 KB, allocated in BSS)
static unsigned char g_walk_work_mem[40960];

// Helper to store path strings for tests - each path gets its own storage
static char test_path_strings[16][256];
static const char *test_path_components[16][MAX_PATH_COMPONENTS];
static int test_path_string_index = 0;

// Helper function to create Path from string for testing
static Path make_path_from_string(const char *str)
{
	Path path;

	// Copy string to test storage
	if (test_path_string_index >= 16)
		test_path_string_index = 0;
	strncpy(test_path_strings[test_path_string_index], str, 255);
	test_path_strings[test_path_string_index][255] = '\0';

	// Parse components and replace separators with nulls
	char *path_str = test_path_strings[test_path_string_index];
	int comp_idx = 0;
	char *comp_start = path_str;

	for (char *p = path_str;; p++) {
		bool at_end = (*p == '\0');
		bool is_sep = (*p == '/' || *p == '\\');

		if (is_sep || at_end) {
			if (p > comp_start && comp_idx < MAX_PATH_COMPONENTS) {
				*p = '\0'; // Terminate component
				test_path_components[test_path_string_index][comp_idx++] =
					comp_start;
			}
			if (at_end)
				break;
			comp_start = p + 1;
		}
	}

	path.components = test_path_components[test_path_string_index];
	path.count = comp_idx;
	path.is_absolute =
		(path_str[0] == '/' || (path_str[1] == ':' && path_str[2] == '/'));

	test_path_string_index++;
	return path;
}

// Helper function to check if directory exists using stdlib
static int directory_exists(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0) {
		return 0;
	}
	return S_ISDIR(st.st_mode);
}

static void remove_dir(const char *path)
{
	rmdir(path);
}

// Helper: create a small file with dummy content
static void create_test_file(const char *path)
{
	FILE *f = fopen(path, "wb");
	if (f) {
		fwrite("test", 1, 4, f);
		fclose(f);
	}
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
	Path path = make_path_from_string(test_path);

	// Remove if exists from previous run
	remove_dir(test_path);

	// Create directory
	ErrorResult result = fun_filesystem_create_directory(path);

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
	Path path = make_path_from_string(test_path);

	// Clean up from previous run
	remove_dir("test_output/nested/level1/level2");
	remove_dir("test_output/nested/level1");
	remove_dir("test_output/nested");

	// Create nested directories
	ErrorResult result = fun_filesystem_create_directory(path);

	// Verify all levels exist
	int exists = directory_exists("test_output/nested") &&
				 directory_exists("test_output/nested/level1") &&
				 directory_exists("test_output/nested/level1/level2");

	if (result.code == ERROR_CODE_NO_ERROR && exists) {
		remove_dir("test_output/nested/level1/level2");
		remove_dir("test_output/nested/level1/level2");
		remove_dir("test_output/nested/level1");
		remove_dir("test_output/nested");
		print_test_result("fun_filesystem_create_directory_nested", 1);
	} else {
		print_test_result("fun_filesystem_create_directory_nested", 0);
	}
}

// Test: fun_filesystem_create_directory idempotent
static void test_fun_filesystem_create_directory_idempotent(void)
{
	const char *test_path = "test_output/idempotent_dir";
	Path path = make_path_from_string(test_path);

	remove_dir(test_path);

	// Create twice - should succeed both times
	fun_filesystem_create_directory(path);
	ErrorResult result = fun_filesystem_create_directory(path);

	if (result.code == ERROR_CODE_NO_ERROR) {
		remove_dir(test_path);
		print_test_result("fun_filesystem_create_directory_idempotent", 1);
	} else {
		print_test_result("fun_filesystem_create_directory_idempotent", 0);
	}
}

// Test: fun_filesystem_create_directory invalid
static void test_fun_filesystem_create_directory_invalid(void)
{
	// NULL path
	Path null_path;
	null_path.components = NULL;
	null_path.count = 0;
	null_path.is_absolute = false;

	ErrorResult result = fun_filesystem_create_directory(null_path);

	if (result.code != ERROR_CODE_NO_ERROR) {
		print_test_result("fun_filesystem_create_directory_null", 1);
	} else {
		print_test_result("fun_filesystem_create_directory_null", 0);
	}
}

// Test: fun_filesystem_remove_directory
static void test_fun_filesystem_remove_directory(void)
{
	const char *test_path = "test_output/rem_dir_test";
	Path path = make_path_from_string(test_path);

	// Clean start using platform API
	rmdir(test_path);
	mkdir("test_output");

	// Create and verify it succeeded
	ErrorResult create_result = fun_filesystem_create_directory(path);
	if (create_result.code != ERROR_CODE_NO_ERROR) {
		print_test_result("fun_filesystem_remove_directory", 0);
		return;
	}

	// Verify directory exists
	if (!directory_exists(test_path)) {
		print_test_result("fun_filesystem_remove_directory", 0);
		return;
	}

	// Remove
	ErrorResult result = fun_filesystem_remove_directory(path);

	if (result.code == ERROR_CODE_NO_ERROR && !directory_exists(test_path)) {
		print_test_result("fun_filesystem_remove_directory", 1);
	} else {
		print_test_result("fun_filesystem_remove_directory", 0);
	}
}

// Test: fun_filesystem_remove_directory not found
static void test_fun_filesystem_remove_directory_not_found(void)
{
	Path path = make_path_from_string("test_output/does_not_exist");

	ErrorResult result = fun_filesystem_remove_directory(path);

	if (result.code == ERROR_CODE_DIRECTORY_NOT_FOUND) {
		print_test_result("fun_filesystem_remove_directory_not_found", 1);
	} else {
		print_test_result("fun_filesystem_remove_directory_not_found", 0);
	}
}

// Test: fun_filesystem_list_directory — verify output is TSV format
static void test_fun_filesystem_list_directory(void)
{
	const char *test_path = "test_output/list_test";
	Path path = make_path_from_string(test_path);

	remove_dir(test_path);
	fun_filesystem_create_directory(path);

	// Create one subdirectory so the listing has an entry to check
	fun_filesystem_create_directory(
		make_path_from_string("test_output/list_test/subdir"));

	MemoryResult buffer_result = fun_memory_allocate(4096);
	if (fun_error_is_error(buffer_result.error)) {
		print_test_result("fun_filesystem_list_directory", 0);
		return;
	}
	Memory buffer = buffer_result.value;
	ErrorResult result = fun_filesystem_list_directory(path, buffer);

	if (result.code == ERROR_CODE_NO_ERROR) {
		// Verify TSV format: output must contain a tab character
		const char *buf = (const char *)buffer;
		int has_tab = 0;
		for (size_t i = 0; buf[i] != '\0'; i++) {
			if (buf[i] == '\t') {
				has_tab = 1;
				break;
			}
		}
		fun_memory_free(&buffer);
		remove_dir("test_output/list_test/subdir");
		remove_dir(test_path);
		print_test_result("fun_filesystem_list_directory", has_tab);
	} else {
		fun_memory_free(&buffer);
		print_test_result("fun_filesystem_list_directory", 0);
	}
}

// Test: fun_path_join
static void test_fun_path_join(void)
{
	Path base, relative, output;
	const char *base_components[] = { "home", "user" };
	const char *rel_components[] = { "documents" };
	const char *out_components[MAX_PATH_COMPONENTS];

	base.components = base_components;
	base.count = 2;
	base.is_absolute = true;

	relative.components = rel_components;
	relative.count = 1;
	relative.is_absolute = false;

	output.components = out_components;

	ErrorResult result = fun_path_join(base, relative, &output);

	if (result.code == ERROR_CODE_NO_ERROR && output.count == 3) {
		print_test_result("fun_path_join", 1);
	} else {
		print_test_result("fun_path_join", 0);
	}
}

// Test: fun_path_normalize
static void test_fun_path_normalize(void)
{
	Path path, output;
	const char *path_components[] = { "home", ".", "user", ".", "docs" };
	const char *out_components[MAX_PATH_COMPONENTS];

	path.components = path_components;
	path.count = 5;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_normalize(path, &output);

	// Should remove . components
	if (result.code == ERROR_CODE_NO_ERROR && output.count == 3) {
		print_test_result("fun_path_normalize", 1);
	} else {
		print_test_result("fun_path_normalize", 0);
	}
}

// Test: fun_path_get_parent
static void test_fun_path_get_parent(void)
{
	Path path, output;
	const char *path_components[] = { "home", "user", "documents", "file.txt" };
	const char *out_components[MAX_PATH_COMPONENTS];

	path.components = path_components;
	path.count = 4;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_get_parent(path, &output);

	if (result.code == ERROR_CODE_NO_ERROR && output.count == 3) {
		print_test_result("fun_path_get_parent", 1);
	} else {
		print_test_result("fun_path_get_parent", 0);
	}
}

// Test: fun_path_get_filename
static void test_fun_path_get_filename(void)
{
	Path path, output;
	const char *path_components[] = { "home", "user", "documents", "file.txt" };
	const char *out_components[1];

	path.components = path_components;
	path.count = 4;
	path.is_absolute = true;

	output.components = out_components;

	ErrorResult result = fun_path_get_filename(path, &output);

	if (result.code == ERROR_CODE_NO_ERROR && output.count == 1) {
		print_test_result("fun_path_get_filename", 1);
	} else {
		print_test_result("fun_path_get_filename", 0);
	}
}

// Test: fun_path_from_string and fun_path_to_string roundtrip
static void test_fun_path_roundtrip(void)
{
	char test_str[] = "/home/user/documents";
	Path path;
	const char *components[MAX_PATH_COMPONENTS];
	path.components = components;

	fun_path_from_string(test_str, &path);

	char output[512];
	ErrorResult result = fun_path_to_string(path, output, sizeof(output));

	// "/home/user/documents" has 3 components: home, user, documents
	if (result.code == ERROR_CODE_NO_ERROR && path.is_absolute &&
		path.count == 3) {
		print_test_result("fun_path_roundtrip", 1);
	} else {
		print_test_result("fun_path_roundtrip", 0);
	}
}

// Test: NULL parameters
static void test_fun_filesystem_null_parameters(void)
{
	// NULL Path components
	Path null_path;
	null_path.components = NULL;
	null_path.count = 0;
	null_path.is_absolute = false;

	ErrorResult result = fun_filesystem_create_directory(null_path);

	// NULL output for path join
	Path base, relative, output;
	base.components = NULL;
	base.count = 0;
	relative.components = NULL;
	relative.count = 0;
	output.components = NULL;

	result = fun_path_join(base, relative, &output);

	if (result.code != ERROR_CODE_NO_ERROR) {
		print_test_result("fun_filesystem_null_parameters", 1);
	} else {
		print_test_result("fun_filesystem_null_parameters", 0);
	}
}

// Test: fun_file_size
static void test_fun_file_size(void)
{
	// Create a small file with known content
	const char *test_path = "test_output/size_test.txt";
	Path path = make_path_from_string(test_path);

	// Write known content using stdlib for setup
	FILE *f = fopen(test_path, "wb");
	if (f == NULL) {
		print_test_result("fun_file_size", 0);
		return;
	}
	fwrite("hello", 1, 5, f);
	fclose(f);

	uint64_t size = 0;
	voidResult result = fun_file_size(path, &size);

	remove(test_path);

	if (fun_error_is_ok(result.error) && size == 5) {
		print_test_result("fun_file_size", 1);
	} else {
		print_test_result("fun_file_size", 0);
	}
}

// Test: fun_file_size on non-existent file
static void test_fun_file_size_not_found(void)
{
	Path path = make_path_from_string("test_output/does_not_exist.txt");
	uint64_t size = 0;
	voidResult result = fun_file_size(path, &size);

	if (fun_error_is_error(result.error)) {
		print_test_result("fun_file_size_not_found", 1);
	} else {
		print_test_result("fun_file_size_not_found", 0);
	}
}

// Test: fun_file_size with null output
static void test_fun_file_size_null_output(void)
{
	Path path = make_path_from_string("test_output/size_test.txt");
	voidResult result = fun_file_size(path, NULL);

	if (fun_error_is_error(result.error)) {
		print_test_result("fun_file_size_null_output", 1);
	} else {
		print_test_result("fun_file_size_null_output", 0);
	}
}

// ===================================================================
// Walk Tests (6.1 - 6.7)
// ===================================================================

// 6.1: Create test tree under test_output/walk_test/
//   file_a.txt, file_b.txt, sub/file_c.txt, sub/deep/file_d.txt
static void setup_walk_test_tree(void)
{
	fun_filesystem_create_directory(
		make_path_from_string("test_output/walk_test"));
	fun_filesystem_create_directory(
		make_path_from_string("test_output/walk_test/sub"));
	fun_filesystem_create_directory(
		make_path_from_string("test_output/walk_test/sub/deep"));
	fun_filesystem_create_directory(
		make_path_from_string("test_output/walk_test_empty"));
	create_test_file("test_output/walk_test/file_a.txt");
	create_test_file("test_output/walk_test/file_b.txt");
	create_test_file("test_output/walk_test/sub/file_c.txt");
	create_test_file("test_output/walk_test/sub/deep/file_d.txt");
}

static void cleanup_walk_test_tree(void)
{
	remove("test_output/walk_test/sub/deep/file_d.txt");
	remove("test_output/walk_test/sub/file_c.txt");
	remove("test_output/walk_test/file_a.txt");
	remove("test_output/walk_test/file_b.txt");
	remove_dir("test_output/walk_test/sub/deep");
	remove_dir("test_output/walk_test/sub");
	remove_dir("test_output/walk_test");
	remove_dir("test_output/walk_test_empty");
}

// 6.2: Walk all entries; expect 4 files and 2 directories
static void test_fun_filesystem_walk_all(void)
{
	Path root = make_path_from_string("test_output/walk_test");
	FunWalkState state;
	ErrorResult init_err =
		fun_filesystem_walk_init(&state, g_walk_work_mem, root);
	if (fun_error_is_error(init_err)) {
		print_test_result("fun_filesystem_walk_all", 0);
		return;
	}

	int file_count = 0, dir_count = 0;
	FileEntry entry;
	boolResult r;
	while ((r = fun_filesystem_walk_next(&state, &entry, false)).value) {
		if (entry.is_directory)
			dir_count++;
		else
			file_count++;
	}
	fun_filesystem_walk_close(&state);

	print_test_result("fun_filesystem_walk_all",
					  file_count == 4 && dir_count == 2);
}

// 6.3: Skip every directory encountered; verify no depth > 0 entries seen
static void test_fun_filesystem_walk_skip_children(void)
{
	Path root = make_path_from_string("test_output/walk_test");
	FunWalkState state;
	ErrorResult init_err =
		fun_filesystem_walk_init(&state, g_walk_work_mem, root);
	if (fun_error_is_error(init_err)) {
		print_test_result("fun_filesystem_walk_skip_children", 0);
		return;
	}

	int found_sub_children = 0;
	int total = 0;
	FileEntry entry;
	bool skip = false;
	boolResult r;
	while ((r = fun_filesystem_walk_next(&state, &entry, skip)).value) {
		total++;
		if (entry.is_directory) {
			skip = true; // Skip children of this directory
		} else {
			if (entry.depth > 0)
				found_sub_children = 1;
			skip = false;
		}
	}
	fun_filesystem_walk_close(&state);

	// Expect 3 entries at depth 0: file_a.txt, file_b.txt, sub/
	print_test_result("fun_filesystem_walk_skip_children",
					  found_sub_children == 0 && total == 3);
}

// 6.4: Skip directories at depth >= 1; verify no entries at depth >= 2 seen
static void test_fun_filesystem_walk_depth_limit(void)
{
	Path root = make_path_from_string("test_output/walk_test");
	FunWalkState state;
	ErrorResult init_err =
		fun_filesystem_walk_init(&state, g_walk_work_mem, root);
	if (fun_error_is_error(init_err)) {
		print_test_result("fun_filesystem_walk_depth_limit", 0);
		return;
	}

	int found_deep = 0; // any entry at depth >= 2
	FileEntry entry;
	bool skip = false;
	boolResult r;
	while ((r = fun_filesystem_walk_next(&state, &entry, skip)).value) {
		if (entry.depth >= 2)
			found_deep = 1;
		skip = (entry.is_directory && entry.depth >= 1);
	}
	fun_filesystem_walk_close(&state);

	print_test_result("fun_filesystem_walk_depth_limit", found_deep == 0);
}

// 6.5: Walk an empty directory; first walk_next returns false
static void test_fun_filesystem_walk_empty(void)
{
	Path root = make_path_from_string("test_output/walk_test_empty");
	FunWalkState state;
	ErrorResult init_err =
		fun_filesystem_walk_init(&state, g_walk_work_mem, root);
	if (fun_error_is_error(init_err)) {
		print_test_result("fun_filesystem_walk_empty", 0);
		return;
	}

	FileEntry entry;
	boolResult r = fun_filesystem_walk_next(&state, &entry, false);
	fun_filesystem_walk_close(&state);

	print_test_result("fun_filesystem_walk_empty", r.value == false);
}

// 6.6: Init, read one entry, call close; must not crash
static void test_fun_filesystem_walk_close(void)
{
	Path root = make_path_from_string("test_output/walk_test");
	FunWalkState state;
	ErrorResult init_err =
		fun_filesystem_walk_init(&state, g_walk_work_mem, root);
	if (fun_error_is_error(init_err)) {
		print_test_result("fun_filesystem_walk_close", 0);
		return;
	}

	FileEntry entry;
	fun_filesystem_walk_next(&state, &entry, false); // read one entry
	fun_filesystem_walk_close(&state); // early close

	print_test_result("fun_filesystem_walk_close", 1);
}

// 6.7: Null args to walk_init return error
static void test_fun_filesystem_walk_null(void)
{
	Path root = make_path_from_string("test_output/walk_test");

	// NULL state pointer
	ErrorResult r1 = fun_filesystem_walk_init(NULL, g_walk_work_mem, root);
	int null_state_ok = fun_error_is_error(r1);

	// NULL work memory
	FunWalkState state;
	ErrorResult r2 = fun_filesystem_walk_init(&state, NULL, root);
	int null_mem_ok = fun_error_is_error(r2);

	// NULL components
	Path null_root;
	null_root.components = NULL;
	null_root.count = 0;
	null_root.is_absolute = false;
	ErrorResult r3 =
		fun_filesystem_walk_init(&state, g_walk_work_mem, null_root);
	int null_root_ok = fun_error_is_error(r3);

	print_test_result("fun_filesystem_walk_null",
					  null_state_ok && null_mem_ok && null_root_ok);
}

// Setup and cleanup
static void setup_tests(void)
{
	fun_filesystem_create_directory(make_path_from_string("test_output"));
	setup_walk_test_tree();
}

static void cleanup_tests(void)
{
	remove_dir("test_output/basic_dir");
	remove_dir("test_output/idempotent_dir");
	remove_dir("test_output/rem_dir_test");
	remove_dir("test_output/list_test");
	remove_dir("test_output/nested/level1/level2");
	remove_dir("test_output/nested/level1");
	remove_dir("test_output/nested");
	cleanup_walk_test_tree();
	remove_dir("test_output");
}

int main(void)
{
	printf("=== Filesystem Module Tests (Path Type API) ===\n\n");

	setup_tests();

	// Directory operations
	test_fun_filesystem_create_directory();
	test_fun_filesystem_create_directory_nested();
	test_fun_filesystem_create_directory_idempotent();
	test_fun_filesystem_create_directory_invalid();
	test_fun_filesystem_remove_directory();
	test_fun_filesystem_remove_directory_not_found();
	test_fun_filesystem_list_directory();

	// Path operations
	test_fun_path_join();
	test_fun_path_normalize();
	test_fun_path_get_parent();
	test_fun_path_get_filename();
	test_fun_path_roundtrip();

	// File metadata
	test_fun_file_size();
	test_fun_file_size_not_found();
	test_fun_file_size_null_output();

	// Error handling
	test_fun_filesystem_null_parameters();

	// Walk tests
	test_fun_filesystem_walk_all();
	test_fun_filesystem_walk_skip_children();
	test_fun_filesystem_walk_depth_limit();
	test_fun_filesystem_walk_empty();
	test_fun_filesystem_walk_close();
	test_fun_filesystem_walk_null();

	cleanup_tests();

	printf("\nTests completed.\n");
	return 0;
}
