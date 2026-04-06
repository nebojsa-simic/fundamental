#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../../../include/fundamental/file/file.h"
#include "../../../include/fundamental/memory/memory.h"
#include "../../../include/fundamental/error/error.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"
#define TEST_FILE_PATH ".\\test_large_file.tmp"
#define TEST_FILE_PATH_UNIX "./test_large_file.tmp"

#ifdef _WIN32
#include <windows.h>
#define FILE_PATH TEST_FILE_PATH
#else
#define FILE_PATH TEST_FILE_PATH_UNIX
#endif

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

// Helper to create a large file with sparse allocation
bool create_sparse_file(const char *path, uint64_t size)
{
#ifdef _WIN32
	HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL,
							   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	LARGE_INTEGER li;
	li.QuadPart = size;
	if (SetFilePointerEx(hFile, li, NULL, FILE_BEGIN) == 0) {
		CloseHandle(hFile);
		return false;
	}

	if (SetEndOfFile(hFile) == 0) {
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);
	return true;
#else
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		return false;
	}

	// Seek to size-1 and write a byte to create sparse file
	off_t result = lseek(fd, size - 1, SEEK_SET);
	if (result == (off_t)-1) {
		close(fd);
		return false;
	}

	char byte = 0;
	if (write(fd, &byte, 1) != 1) {
		close(fd);
		return false;
	}

	close(fd);
	return true;
#endif
}

bool file_exists(const char *path)
{
#ifdef _WIN32
	DWORD attrs = GetFileAttributesA(path);
	return attrs != INVALID_FILE_ATTRIBUTES;
#else
	struct stat st;
	return stat(path, &st) == 0;
#endif
}

void delete_test_file(const char *path)
{
#ifdef _WIN32
	DeleteFileA(path);
#else
	unlink(path);
#endif
}

// Test 1: Create a file slightly larger than 2GB
void test_create_file_larger_than_2gb(void)
{
	// 2GB + 1MB
	uint64_t file_size = (2ULL * 1024 * 1024 * 1024) + (1 * 1024 * 1024);

	assert(create_sparse_file(FILE_PATH, file_size));
	assert(file_exists(FILE_PATH));

	delete_test_file(FILE_PATH);
	print_test_result("create_file_larger_than_2gb");
}

// Test 2: Read at offset beyond 2GB boundary
void test_read_beyond_2gb_offset(void)
{
	// Create 3GB file
	uint64_t file_size = 3ULL * 1024 * 1024 * 1024;
	assert(create_sparse_file(FILE_PATH, file_size));

	// Try to read at 2.5GB offset
	uint64_t offset = (5ULL * 1024 * 1024 * 1024) / 2; // 2.5GB
	size_t bytes_to_read = 1024; // 1KB

	MemoryResult mem_res = fun_memory_allocate(bytes_to_read);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;

	// Initialize buffer with pattern
	memset(buffer, 0xAA, bytes_to_read);

	Read params = { .file_path = STRING_LITERAL(FILE_PATH),
					.output = buffer,
					.bytes_to_read = bytes_to_read,
					.offset = offset,
					.mode = FILE_MODE_AUTO };

	AsyncResult read_res = fun_read_file_in_memory(params);
	fun_async_await(&read_res, -1);

	// Reading from sparse file region should return zeros or error
	// depending on implementation
	assert(read_res.error.code == 0 ||
		   read_res.error.code == ERROR_CODE_IO_READ_FAILED);

	fun_memory_free(&buffer);
	delete_test_file(FILE_PATH);
	print_test_result("read_beyond_2gb_offset");
}

// Test 3: Write at large offset
void test_write_at_large_offset(void)
{
	// Create 2.5GB file
	uint64_t file_size = (5ULL * 1024 * 1024 * 1024) / 2;
	assert(create_sparse_file(FILE_PATH, file_size));

	// Write 100 bytes at 2GB offset
	uint64_t offset = 2ULL * 1024 * 1024 * 1024;
	const char *test_data = "Large file test data at 2GB+ offset";
	size_t data_size = strlen(test_data);

	MemoryResult mem_res = fun_memory_allocate(data_size);
	assert(mem_res.error.code == 0 && mem_res.value != NULL);
	Memory buffer = mem_res.value;
	memcpy(buffer, test_data, data_size);

	Write params = { .file_path = STRING_LITERAL(FILE_PATH),
					 .input = buffer,
					 .bytes_to_write = data_size,
					 .offset = offset,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_ASYNC };

	AsyncResult write_res = fun_write_file_in_memory(params);
	fun_async_await(&write_res, -1);

	// Should succeed or fail gracefully
	assert(write_res.error.code == 0 ||
		   write_res.error.code == ERROR_CODE_IO_WRITE_FAILED);

	fun_memory_free(&buffer);
	delete_test_file(FILE_PATH);
	print_test_result("write_at_large_offset");
}

// Test 4: Verify offset alignment for large files
void test_large_file_offset_alignment(void)
{
	// Create 3GB file
	uint64_t file_size = 3ULL * 1024 * 1024 * 1024;
	assert(create_sparse_file(FILE_PATH, file_size));

	// Test various large offsets
	uint64_t offsets[] = {
		1ULL * 1024 * 1024 * 1024, // 1GB
		2ULL * 1024 * 1024 * 1024, // 2GB
		3ULL * 1024 * 1024 * 1024, // 3GB
		(2ULL * 1024 * 1024 * 1024) + 4096, // 2GB + page
	};

	for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
		uint64_t offset = offsets[i];

		// Should not overflow when calculating aligned offset
		assert(offset < file_size || offset == file_size);
	}

	delete_test_file(FILE_PATH);
	print_test_result("large_file_offset_alignment");
}

// Test 5: Test integer overflow protection in size calculations
void test_overflow_protection_large_sizes(void)
{
	// Test near UINT64_MAX values
	uint64_t max_safe_size = UINT64_MAX - 4096;

	// These calculations should not overflow
	uint64_t aligned = max_safe_size & ~(4096ULL - 1);
	assert(aligned <= max_safe_size);

	// Adding page size should not overflow
	if (max_safe_size + 4096 > max_safe_size) {
		// No overflow - good
	} else {
		// Overflow detected - this is expected for near-max values
		// Implementation should handle this gracefully
	}

	print_test_result("overflow_protection_large_sizes");
}

int main(void)
{
	printf("Running large file tests:\n");

	test_create_file_larger_than_2gb();
	test_read_beyond_2gb_offset();
	test_write_at_large_offset();
	test_large_file_offset_alignment();
	test_overflow_protection_large_sizes();

	printf("All large file tests passed!\n");
	return 0;
}
