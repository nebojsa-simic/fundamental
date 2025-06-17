#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

// Include headers for the fundamental library components
#include "../../include/file/file.h"
#include "../../include/memory/memory.h"
#include "../../include/string/string.h"
#include "../../include/error/error.h"
#include "../../include/async/async.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define TEST_FILENAME "test_write_file.tmp"
#define TEST_DIR "./"
#define TEST_FILE_PATH ".\\test_write_file.tmp"

#include <windows.h>

typedef struct {
	Write parameters;
	HANDLE file_handle;
	HANDLE mapping_handle;
	LPVOID mapped_view;
	uint64_t adjusted_offset;
	uint64_t original_file_size;
	bool file_extended;
} MMapWriteState;

// --- Mock/Simplified Async Handling for Testing ---
#define GET_FINAL_RESULT(async_result) (async_result.error)
#define ASSERT_NO_ERROR(result) assert(GET_FINAL_RESULT(result).code == 0)
#define ASSERT_ERROR(result) assert(GET_FINAL_RESULT(result).code != 0)

// Helper function to print test progress
void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

// Helper function to compare two memory regions
int memoryCompare(const void *s1, const void *s2, size_t n)
{
	const unsigned char *p1 = s1, *p2 = s2;
	for (size_t i = 0; i < n; i++) {
		if (p1[i] != p2[i]) {
			printf("memoryCompare %x != %x for %d\n", p1[i], p2[i], i);
			return p1[i] < p2[i] ? -1 : 1;
		}
	}
	return 0;
}

// --- Test File Helper Functions ---

// Reads a file and returns its content in a newly allocated buffer
bool read_file_content(const char *path, char **content, size_t *size)
{
    int fd = open(path, O_RDONLY | O_BINARY);
    if (fd == -1) {
        printf("Failed to open file for reading: %s\n", path);
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        printf("Failed to get file size: %s\n", path);
        return false;
    }

    *size = st.st_size;
    *content = malloc(*size);
    if (!*content) {
        close(fd);
        printf("Failed to allocate memory for file content: %s\n", path);
        return false;
    }

    // Read file content in a loop until all bytes are read
    size_t total_bytes_read = 0;
    while (total_bytes_read < *size) {
        ssize_t bytes_read = read(fd, (char*)*content + total_bytes_read, 
                                 *size - total_bytes_read);
        
        if (bytes_read == -1) {
            if (errno == EINTR) {
                // Interrupted by signal - retry
                continue;
            }
            // Real error occurred
            close(fd);
            free(*content);
            printf("Failed to read file content: %s, error: %s\n", 
                   path, strerror(errno));
            return false;
        }
        
        if (bytes_read == 0) {
            // Unexpected EOF - file was truncated during read
            close(fd);
            free(*content);
            printf("Unexpected EOF while reading file: %s, read %zu of %zu bytes\n", 
                   path, total_bytes_read, *size);
            return false;
        }
        
        total_bytes_read += bytes_read;
    }

    close(fd);
    return true;
}

// Creates a test file with specified content
bool create_test_file(const char *path, const char *content,
					  size_t content_size)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("Failed to create test file (open)");
		return false;
	}

	if (content_size > 0) {
		ssize_t written = write(fd, content, content_size);
		if (written == -1 || (size_t)written != content_size) {
			perror("Failed to create test file (write)");
			close(fd);
			unlink(path);
			return false;
		}
	}

	if (close(fd) == -1) {
		perror("Failed to create test file (close)");
		unlink(path);
		return false;
	}

	return true;
}

// Deletes the test file
void delete_test_file(const char *path)
{
	if (unlink(path) != 0 && errno != ENOENT) {
		perror("Warning: Failed to delete test file");
	}
}

// --- Test Cases for fun_write_memory_to_file ---

void test_fun_write_memory_to_file_success()
{
	const char *content = "Hello Fundamental Write World!";
	size_t content_size = strlen(content);

	// Allocate memory for input data
	MemoryResult mem_res = fun_memory_allocate(content_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, content, content_size);

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = content_size,
					 .offset = 0,
					 .mode = FILE_MODE_AUTO };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_NO_ERROR(write_res);

	// Verify file was written correctly
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));
	assert(file_size == content_size);
	assert(memoryCompare(file_content, content, content_size) == 0);

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_success");
}

void test_fun_write_memory_to_file_with_offset()
{
	const char *initial_content = "0123456789ABCDEF";
	const char *new_content = "WRITE";
	size_t initial_size = strlen(initial_content);
	size_t new_size = strlen(new_content);
	uint64_t offset = 5;

	// Create initial file
	assert(create_test_file(TEST_FILE_PATH, initial_content, initial_size));

	// Allocate memory for new content
	MemoryResult mem_res = fun_memory_allocate(new_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, new_content, new_size);

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = new_size,
					 .offset = offset };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_NO_ERROR(write_res);

	// Verify file content
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));
	assert(file_size == initial_size); // File size shouldn't change

	// Check that data was written at correct offset
	assert(memoryCompare(file_content, "01234", 5) ==
		   0); // Before offset unchanged
	assert(memoryCompare(file_content + offset, "WRITE", new_size) ==
		   0); // New content at offset
	assert(memoryCompare(file_content + offset + new_size, "ABCDEF", 6) ==
		   0); // After unchanged

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_with_offset");
}

void test_fun_write_memory_to_file_extend_file()
{
	const char *initial_content = "Short";
	const char *new_content = "This extends the file beyond its original size";
	size_t initial_size = strlen(initial_content);
	size_t new_size = strlen(new_content);
	uint64_t offset = 10; // Beyond current file size

	// Create initial file
	assert(create_test_file(TEST_FILE_PATH, initial_content, initial_size));

	// Allocate memory for new content
	MemoryResult mem_res = fun_memory_allocate(new_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, new_content, new_size);

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = new_size,
					 .offset = offset };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_NO_ERROR(write_res);

	// Verify file was extended
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));
	assert(file_size == offset + new_size);

	// Check original content
	assert(memoryCompare(file_content, initial_content, initial_size) == 0);
	// Check gap is zero-filled (implementation dependent)
	// Check new content at offset
	assert(memoryCompare(file_content + offset, new_content, new_size) == 0);

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_extend_file");
}

void test_fun_write_memory_to_file_new_file()
{
	const char *content = "Creating a new file from scratch!";
	size_t content_size = strlen(content);

	// Ensure file doesn't exist
	delete_test_file(TEST_FILE_PATH);

	// Allocate memory for content
	MemoryResult mem_res = fun_memory_allocate(content_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, content, content_size);

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = content_size,
					 .offset = 0 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_NO_ERROR(write_res);

	// Verify file was created and written
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));
	assert(file_size == content_size);
	assert(memoryCompare(file_content, content, content_size) == 0);

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_new_file");
}

void test_fun_write_memory_to_file_zero_bytes()
{
	const char *initial_content = "Don't change this content";
	size_t initial_size = strlen(initial_content);

	// Create initial file
	assert(create_test_file(TEST_FILE_PATH, initial_content, initial_size));

	// Allocate minimal memory (even for zero bytes)
	MemoryResult mem_res = fun_memory_allocate(1);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = 0,
					 .offset = 5 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_NO_ERROR(write_res); // Writing zero bytes should succeed

	// Verify file content is unchanged
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));
	assert(file_size == initial_size);
	assert(memoryCompare(file_content, initial_content, initial_size) == 0);

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_zero_bytes");
}

void test_fun_write_memory_to_file_overwrite_complete()
{
	const char *initial_content = "Replace this entirely";
	const char *new_content = "Completely new content!";
	size_t initial_size = strlen(initial_content);
	size_t new_size = strlen(new_content);

	// Create initial file
	assert(create_test_file(TEST_FILE_PATH, initial_content, initial_size));

	// Allocate memory for new content
	MemoryResult mem_res = fun_memory_allocate(new_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, new_content, new_size);

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = new_size,
					 .offset = 0 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_NO_ERROR(write_res);

	// Verify file content
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));

	// File size should be max of original and new content
	size_t expected_size = (new_size > initial_size) ? new_size : initial_size;
	assert(file_size == expected_size);
	assert(memoryCompare(file_content, new_content, new_size) == 0);

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_overwrite_complete");
}

void test_fun_write_memory_to_file_large_data()
{
	const size_t large_size = 1024 * 1024; // 1MB

	// Create large data pattern
	MemoryResult mem_res = fun_memory_allocate(large_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;

	// Fill with pattern
	for (size_t i = 0; i < large_size; i++) {
		((char *)input_buffer)[i] = (char)(i % 256);
	}

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = large_size,
					 .offset = 0 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_NO_ERROR(write_res);

	// Verify file content
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));
	printf("%d", file_size);
	printf("%d", large_size);
	assert(file_size == large_size);
	assert(memoryCompare(file_content, input_buffer, large_size) == 0);

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_large_data");
}

// --- Error Case Tests ---

void test_fun_write_memory_to_file_null_input_buffer()
{
	Memory null_buffer = NULL;

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = null_buffer,
					 .bytes_to_write = 10,
					 .offset = 0 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_ERROR(write_res);

	print_test_result("fun_write_memory_to_file_null_input_buffer");
}

void test_fun_write_memory_to_file_null_file_path()
{
	const char *content = "Test content";
	size_t content_size = strlen(content);

	MemoryResult mem_res = fun_memory_allocate(content_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, content, content_size);

	String null_path = { 0 }; // Or however null string is represented

	Write params = { .file_path = null_path,
					 .input = input_buffer,
					 .bytes_to_write = content_size,
					 .offset = 0 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_ERROR(write_res);

	// Cleanup
	fun_memory_free(&input_buffer);
	print_test_result("fun_write_memory_to_file_null_file_path");
}

void test_fun_write_memory_to_file_invalid_path()
{
	const char *content = "Test content";
	size_t content_size = strlen(content);

	MemoryResult mem_res = fun_memory_allocate(content_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, content, content_size);

	// Use invalid path (directory that doesn't exist)
	const char *invalid_path = "/nonexistent/directory/file.txt";

	Write params = { .file_path = invalid_path,
					 .input = input_buffer,
					 .bytes_to_write = content_size,
					 .offset = 0 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_ERROR(write_res);

	// Cleanup
	fun_memory_free(&input_buffer);
	print_test_result("fun_write_memory_to_file_invalid_path");
}

void test_fun_write_memory_to_file_readonly_file()
{
	const char *initial_content = "Read only content";
	const char *new_content = "Should not be written";
	size_t initial_size = strlen(initial_content);
	size_t new_size = strlen(new_content);

	// Create file and make it read-only
	assert(create_test_file(TEST_FILE_PATH, initial_content, initial_size));
	chmod(TEST_FILE_PATH, S_IRUSR); // Read-only permission

	MemoryResult mem_res = fun_memory_allocate(new_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory input_buffer = mem_res.value;
	memcpy(input_buffer, new_content, new_size);

	Write params = { .file_path = TEST_FILE_PATH,
					 .input = input_buffer,
					 .bytes_to_write = new_size,
					 .offset = 0 };

	AsyncResult write_res = fun_write_memory_to_file(params);
	fun_async_await(&write_res);
	ASSERT_ERROR(write_res); // Should fail due to read-only permissions

	// Verify original content is unchanged
	chmod(TEST_FILE_PATH, S_IRUSR | S_IWUSR); // Restore permissions for cleanup
	char *file_content;
	size_t file_size;
	assert(read_file_content(TEST_FILE_PATH, &file_content, &file_size));
	assert(file_size == initial_size);
	assert(memoryCompare(file_content, initial_content, initial_size) == 0);

	// Cleanup
	free(file_content);
	fun_memory_free(&input_buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_write_memory_to_file_readonly_file");
}

// --- Main Test Runner ---

int main()
{
	printf("Running file write module tests:\n");

	// Basic functionality tests
	test_fun_write_memory_to_file_success();
	test_fun_write_memory_to_file_with_offset();
	test_fun_write_memory_to_file_extend_file();
	test_fun_write_memory_to_file_new_file();
	test_fun_write_memory_to_file_zero_bytes();
	test_fun_write_memory_to_file_overwrite_complete();
	test_fun_write_memory_to_file_large_data();

	// Error case tests
	test_fun_write_memory_to_file_null_input_buffer();
	test_fun_write_memory_to_file_null_file_path();
	test_fun_write_memory_to_file_invalid_path();
	test_fun_write_memory_to_file_readonly_file();

	printf("All file write tests passed!\n");
	return 0;
}
