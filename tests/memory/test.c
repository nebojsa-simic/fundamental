#include <assert.h>
#include <stdio.h>
#include "../../include/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

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
			return p1[i] - p2[i];
		}
	}
	return 0;
}

void test_fun_memory_allocate()
{
	// Test successful allocation
	size_t size = 1024;
	MemoryResult result = fun_memory_allocate(size);
	ASSERT_NO_ERROR(result);
	assert(result.value != NULL);

	// Clean up
	voidResult freeResult = fun_memory_free(&result.value);
	ASSERT_NO_ERROR(freeResult);

	// Test allocation of size 0 (should succeed with minimum allocation)
	result = fun_memory_allocate(0);
	ASSERT_NO_ERROR(result);
	assert(result.value != NULL);

	// Clean up the zero-size allocation
	freeResult = fun_memory_free(&result.value);
	ASSERT_NO_ERROR(freeResult);

	// Test very large allocation (might fail depending on available memory)
	result = fun_memory_allocate((size_t)-1);
	ASSERT_ERROR(result);
	assert(result.value == NULL);

	print_test_result("fun_memory_allocate");
}

void test_fun_memory_reallocate()
{
	// Allocate initial memory
	size_t initialSize = 1024;
	MemoryResult result = fun_memory_allocate(initialSize);
	ASSERT_NO_ERROR(result);
	Memory memory = result.value;

	// Test successful reallocation (increase size)
	size_t newSize = 2048;
	result = fun_memory_reallocate(memory, newSize);
	ASSERT_NO_ERROR(result);
	assert(result.value != NULL);

	// Test reallocation to smaller size
	newSize = 512;
	result = fun_memory_reallocate(result.value, newSize);
	ASSERT_NO_ERROR(result);
	assert(result.value != NULL);

	// Clean up
	voidResult freeResult = fun_memory_free(&result.value);
	ASSERT_NO_ERROR(freeResult);

	// Test reallocation of NULL pointer (should act like memoryAllocate)
	result = fun_memory_reallocate(NULL, 1024);
	ASSERT_NO_ERROR(result);
	assert(result.value == NULL);

	// Clean up
	freeResult = fun_memory_free(&result.value);
	ASSERT_NO_ERROR(freeResult);

	print_test_result("fun_memory_reallocate");
}

void test_fun_memory_free()
{
	// Allocate memory to free
	MemoryResult allocResult = fun_memory_allocate(1024);
	ASSERT_NO_ERROR(allocResult);
	Memory memory = allocResult.value;

	// Test successful free
	voidResult freeResult = fun_memory_free(&memory);
	ASSERT_NO_ERROR(freeResult);
	assert(memory == NULL);

	// Test double free
	freeResult = fun_memory_free(&memory);
	ASSERT_NO_ERROR(freeResult);
	assert(memory == NULL);

	// Test freeing NULL pointer (should not cause an error)
	memory = NULL;
	freeResult = fun_memory_free(&memory);
	ASSERT_NO_ERROR(freeResult);

	print_test_result("fun_memory_free");
}

void test_fun_memory_fill()
{
	// Allocate memory to fill
	size_t size = 1024;
	MemoryResult allocResult = fun_memory_allocate(size);
	ASSERT_NO_ERROR(allocResult);
	Memory memory = allocResult.value;

	// Test successful fill
	uint64_t fillValue = 0xAA;
	voidResult fillResult = fun_memory_fill(memory, size, fillValue);
	ASSERT_NO_ERROR(fillResult);

	// Verify the fill
	for (size_t i = 0; i < (size / sizeof(uint64_t)); i++) {
		assert(((uint64_t *)memory)[i] == fillValue);
	}

	// Test fill with NULL pointer (should fail)
	fillResult = fun_memory_fill(NULL, size, fillValue);
	ASSERT_ERROR(fillResult);

	// Test fill with size 0 (should succeed but do nothing)
	fillResult = fun_memory_fill(memory, 0, fillValue);
	ASSERT_NO_ERROR(fillResult);

	// Clean up
	voidResult freeResult = fun_memory_free(&memory);
	ASSERT_NO_ERROR(freeResult);

	print_test_result("fun_memory_fill");
}

void test_fun_memory_size()
{
	// Allocate memory
	size_t allocSize = 1024;
	MemoryResult allocResult = fun_memory_allocate(allocSize);
	ASSERT_NO_ERROR(allocResult);
	Memory memory = allocResult.value;

	// Test getting size of allocated memory
	size_tResult sizeResult = fun_memory_size(memory);
	ASSERT_NO_ERROR(sizeResult);
	assert(sizeResult.value >= allocSize); // May be larger due to alignment

	// Test getting size of NULL pointer (should fail)
	sizeResult = fun_memory_size(NULL);
	ASSERT_ERROR(sizeResult);
	assert(sizeResult.value == 0);

	// Clean up
	voidResult freeResult = fun_memory_free(&memory);
	ASSERT_NO_ERROR(freeResult);

	print_test_result("fun_memory_size");
}

void test_fun_memory_copy()
{
	// Test 1: Basic copy
	char src1[] = "Hello, World!";
	char dest1[20] = { 0 };
	voidResult result = fun_memory_copy(src1, dest1, sizeof(src1));
	ASSERT_NO_ERROR(result);
	assert(memoryCompare(src1, dest1, sizeof(src1)) == 0);

	// Test 2: Copy with offset
	char src2[] = "Ananas!";
	char dest2[20] = "Hello, ------!";
	result = fun_memory_copy(src2, dest2 + 7, 7);
	ASSERT_NO_ERROR(result);
	assert(memoryCompare(dest2, "Hello, Ananas!", 14) == 0);

	// Test 3: Copy zero bytes
	char src3[] = "Test";
	char dest3[10] = { 0 };
	result = fun_memory_copy(src3, dest3, 0);
	ASSERT_NO_ERROR(result);
	assert(dest3[0] == 0);

	// Test 4: Copy to overlapping region (forward)
	char buffer1[] = "abcdefghijklmnop";
	result = fun_memory_copy(buffer1, buffer1 + 4, 8);
	ASSERT_NO_ERROR(result);
	assert(memoryCompare(buffer1, "abcdabcdefghmnop", 16) == 0);

	// Test 5: Copy to overlapping region (backward)
	char buffer2[] = "abcdefghijklmnop";
	result = fun_memory_copy(buffer2 + 4, buffer2, 8);
	ASSERT_NO_ERROR(result);
	assert(memoryCompare(buffer2, "efghijklijklmnop", 16) == 0);

	// Test 6: NULL destination
	result = fun_memory_copy(NULL, src1, sizeof(src1));
	ASSERT_ERROR(result);

	// Test 7: NULL source
	result = fun_memory_copy(dest1, NULL, sizeof(src1));
	ASSERT_ERROR(result);

	print_test_result("fun_memory_copy");
}

int main()
{
	printf("Running memory module tests:\n");
	test_fun_memory_allocate();
	test_fun_memory_reallocate();
	test_fun_memory_free();
	test_fun_memory_fill();
	test_fun_memory_size();
	test_fun_memory_copy();
	printf("All tests passed!\n");
	return 0;
}
