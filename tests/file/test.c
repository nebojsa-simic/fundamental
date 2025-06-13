#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h> // For unlink
#include <fcntl.h> // For open, O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> // For S_IRUSR, S_IWUSR
#include <string.h> // For strerror, strcmp - temporary for helpers, replace if needed
#include <errno.h> // For errno

// Include headers for the fundamental library components being tested or used
#include "../../include/file/file.h"
#include "../../include/memory/memory.h"
#include "../../include/string/string.h"
#include "../../include/error/error.h"
#include "../../include/async/async.h" // Assuming async provides a wait/get result mechanism

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define TEST_FILENAME "test_read_file.tmp"
#define TEST_DIR "./"
#define TEST_FILE_PATH ".\\test_read_file.tmp"

#include <windows.h>

typedef struct {
	Read parameters;
	HANDLE file_handle;
	HANDLE mapping_handle;
	LPVOID mapped_view;
	uint64_t adjusted_offset;
	Memory output_buffer;
} MMapState;

// --- Mock/Simplified Async Handling for Testing ---
// In a real scenario, this would interact with your async subsystem.
// For testing, we might simulate blocking completion or have a test-specific
// synchronous implementation. Here, we assume AsyncResult directly contains
// the final Error or can be waited upon simply.
// Let's assume a function `fun_async_wait_result` exists for testing.
// ErrorResult fun_async_wait_result(AsyncResult async_result); // Placeholder

// For simplicity in this example, we'll assume the AsyncResult returned
// *immediately* contains the final status for testing purposes.
// Adapt this based on your actual async implementation.
#define GET_FINAL_RESULT(async_result) (async_result.error)

// Helper macro to check if an error occurred in the final result
#define ASSERT_NO_ERROR(result) assert(GET_FINAL_RESULT(result).code == 0)
#define ASSERT_ERROR(result) assert(GET_FINAL_RESULT(result).code != 0)

// Helper function to print test progress
void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

// Helper function to compare two memory regions (from example)
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

// Creates a test file with specified content. Returns true on success.
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
			unlink(path); // Attempt cleanup
			return false;
		}
	}
	if (close(fd) == -1) {
		perror("Failed to create test file (close)");
		unlink(path); // Attempt cleanup
		return false;
	}
	return true;
}

// Deletes the test file.
void delete_test_file(const char *path)
{
	if (unlink(path) != 0 && errno != ENOENT) { // Don't warn if already deleted
		perror("Warning: Failed to delete test file");
	}
}

// --- Test Cases for fun_read_file_in_memory ---

void test_fun_read_file_in_memory_success()
{
	const char *content = "Hello Fundamental World!";
	size_t content_size = strlen(content);
	assert(create_test_file(TEST_FILE_PATH, content, content_size));

	MemoryResult mem_res = fun_memory_allocate(content_size + 1);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = {
		.file_path = TEST_FILE_PATH,
		.output = buffer,
		.bytes_to_read = content_size,
		.offset = 0,
		.mode = FILE_MODE_AUTO // Or specific mode if testing that
	};

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	ASSERT_NO_ERROR(read_res);
	assert(memoryCompare(buffer, content, content_size) == 0);
	// Cleanup
	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_success");
}

void test_fun_read_file_in_memory_partial_start()
{
	const char *content = "Read only the first part.";
	size_t content_size = strlen(content);
	size_t bytes_to_read = 10;
	assert(create_test_file(TEST_FILE_PATH, content, content_size));

	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = 0 };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	ASSERT_NO_ERROR(read_res);
	assert(memoryCompare(buffer, content, bytes_to_read) == 0);

	// Cleanup
	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_partial_start");
}

void test_fun_read_file_in_memory_partial_offset()
{
	const char *content = "Start reading from an offset.";
	size_t content_size = strlen(content);
	uint64_t offset = 6; // Start at "reading"
	size_t bytes_to_read = 7; // Read "reading"
	assert(create_test_file(TEST_FILE_PATH, content, content_size));

	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = offset };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	ASSERT_NO_ERROR(read_res);
	assert(memoryCompare(buffer, content + offset, bytes_to_read) == 0);

	// Cleanup
	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_partial_offset");
}

void test_fun_read_file_in_memory_empty_file()
{
	assert(create_test_file(TEST_FILE_PATH, "", 0)); // Create empty file

	size_t bytes_to_read = 10; // Try to read from empty file
	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	// Fill buffer to ensure read doesn't leave old data if it reads 0 bytes
	memset(buffer, 0xAA, bytes_to_read);

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = 0 };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	// Reading past EOF is not necessarily an error, but should read 0 bytes.
	// The function API (.bytes_to_read REQUIRED - Exact bytes to read) implies
	// it might be an error if fewer bytes than requested are read.
	// Let's assume it's an error in this fundamental library context.
	ASSERT_ERROR(read_res);
	// Or, if reading 0 bytes successfully is expected:
	// ASSERT_NO_ERROR(read_res);
	// Check that buffer remains unchanged or check a returned 'bytes_read' value if API provided one.

	// Cleanup
	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_empty_file");
}

void test_fun_read_file_in_memory_read_zero_bytes()
{
	const char *content = "Read zero bytes from this.";
	size_t content_size = strlen(content);
	assert(create_test_file(TEST_FILE_PATH, content, content_size));

	size_t bytes_to_read = 0;
	MemoryResult mem_res = fun_memory_allocate(1); // Allocate dummy byte
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	((char *)buffer)[0] = 'X'; // Mark buffer

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer, // Pass valid buffer even for 0 bytes
					.bytes_to_read = bytes_to_read,
					.offset = 5 };

	AsyncResult read_res = fun_read_file_in_memory(params);
	// Reading zero bytes should succeed and do nothing.
	fun_async_await(&read_res);
	ASSERT_NO_ERROR(read_res);
	assert(((char *)buffer)[0] == 'X'); // Buffer should be untouched

	// Cleanup
	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_read_zero_bytes");
}

void test_fun_read_file_in_memory_offset_beyond_eof()
{
	const char *content = "Offset beyond EOF.";
	size_t content_size = strlen(content);
	assert(create_test_file(TEST_FILE_PATH, content, content_size));

	size_t bytes_to_read = 10;
	uint64_t offset = content_size + 5; // Offset past end of file
	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	memset(buffer, 0xBB, bytes_to_read); // Fill buffer

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = offset };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	// Attempting to read starting past EOF. Should likely fail as exact bytes are required.
	ASSERT_ERROR(read_res);
	// Or, if reading 0 bytes is expected:
	// ASSERT_NO_ERROR(read_res);
	// assert(... buffer is unchanged or check bytes read result ...);

	// Cleanup
	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_offset_beyond_eof");
}

void test_fun_read_file_in_memory_read_beyond_eof()
{
	const char *content = "Read beyond EOF.";
	size_t content_size = strlen(content);
	assert(create_test_file(TEST_FILE_PATH, content, content_size));

	uint64_t offset = 10; // "beyond EOF." (12 chars)
	size_t bytes_to_read = 20; // Request more than available
	size_t actual_available = content_size - offset; // Should be 12

	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = bytes_to_read, // Requesting exact 20 bytes
					.offset = offset };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	// Since the API requires reading the *exact* number of bytes,
	// reading fewer bytes than requested should be an error.
	ASSERT_ERROR(read_res);
	// If the API allowed partial reads, the check would be:
	// ASSERT_NO_ERROR(read_res);
	// // Check that only actual_available bytes were read and buffer content is correct
	// assert(memoryCompare(buffer, content + offset, actual_available) == 0);
	// // Need a way to know how many bytes were *actually* read if API supports it.

	// Cleanup
	fun_memory_free(&buffer);
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_read_beyond_eof");
}

void test_fun_read_file_in_memory_non_existent_file()
{
	delete_test_file(TEST_FILE_PATH); // Ensure it doesn't exist

	size_t bytes_to_read = 10;
	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	Read params = { .file_path = TEST_FILE_PATH,
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = 0 };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	// Should fail because the file doesn't exist.
	ASSERT_ERROR(read_res);

	// Cleanup
	fun_memory_free(&buffer);
	// No need to delete file here
	print_test_result("fun_read_file_in_memory_non_existent_file");
}

void test_fun_read_file_in_memory_null_output_buffer()
{
	const char *content = "Test null buffer.";
	size_t content_size = strlen(content);
	assert(create_test_file(TEST_FILE_PATH, content, content_size));

	// Intentionally set output buffer pointer to NULL
	Memory null_buffer_ptr = NULL;

	Read params = { .file_path = TEST_FILE_PATH,
					.output = null_buffer_ptr, // Pass address of NULL pointer
					.bytes_to_read = 5,
					.offset = 0 };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	// Should fail due to NULL buffer.
	ASSERT_ERROR(read_res);
	assert(null_buffer_ptr ==
		   NULL); // Ensure the pointer itself wasn't modified

	// Test with the pointer *to* the Memory variable being NULL
	// This depends on how NULL checks are done internally.
	// If the function dereferences `parameters.output` before checking
	// the Memory variable it points to, this could crash.
	// If it checks `parameters.output` itself first, it might handle it.
	// Let's assume passing NULL for the 'output' field causes the error.
	Read params_null_output_field = {
		.file_path = TEST_FILE_PATH,
		.output = NULL, // Pass NULL directly for the pointer-to-Memory
		.bytes_to_read = 5,
		.offset = 0
	};
	// This call's behavior depends heavily on implementation details
	// and might even be considered undefined behavior if not handled explicitly.
	// Assuming it's designed to handle `output == NULL` as an error:
	// read_res = fun_read_file_in_memory(params_null_output_field);
	// ASSERT_ERROR(read_res);

	// Cleanup
	delete_test_file(TEST_FILE_PATH);
	print_test_result("fun_read_file_in_memory_null_output_buffer");
}

void test_fun_read_file_in_memory_null_file_path()
{
	// Use a valid buffer setup
	size_t bytes_to_read = 10;
	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	// Create a NULL String (assuming String has identifiable null state)
	String null_path = { 0 }; // Or however a null/invalid string is represented

	Read params = { .file_path = null_path, // Use the null String
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = 0 };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res);
	// Should fail due to null/invalid file path.
	ASSERT_ERROR(read_res);

	// Cleanup
	fun_memory_free(&buffer);
	print_test_result("fun_read_file_in_memory_null_file_path");
}

// --- Main Test Runner ---

int main()
{
	printf("Running file read module tests:\n");

	// Setup (e.g., ensure test directory exists if needed)
	// mkdir(TEST_DIR, 0777); // May need error checking

	test_fun_read_file_in_memory_success();
	test_fun_read_file_in_memory_partial_start();
	test_fun_read_file_in_memory_partial_offset();
	test_fun_read_file_in_memory_empty_file();
	test_fun_read_file_in_memory_read_zero_bytes();
	test_fun_read_file_in_memory_offset_beyond_eof();
	test_fun_read_file_in_memory_read_beyond_eof();
	test_fun_read_file_in_memory_non_existent_file();
	test_fun_read_file_in_memory_null_output_buffer();
	test_fun_read_file_in_memory_null_file_path();

	// Add tests for different FileMode values if their behavior differs significantly
	// e.g., test_fun_read_file_in_memory_mmap_success();

	printf("All file read tests passed!\n");
	return 0;
}
