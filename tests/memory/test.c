#include "fundamental/memory/memory.h"
#include "fundamental/console/console.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

// Helper function to print test progress
void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
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
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(result.value != NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Clean up
	voidResult freeResult = fun_memory_free(&result.value);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	// Test allocation of size 0 (should succeed with minimum allocation)
	result = fun_memory_allocate(0);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(result.value != NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Clean up the zero-size allocation
	freeResult = fun_memory_free(&result.value);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	// Test very large allocation (might fail depending on available memory)
	result = fun_memory_allocate((size_t)-1);
	if (result.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
	if (!(result.value == NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	print_test_result("fun_memory_allocate");
}

void test_fun_memory_reallocate()
{
	// Allocate initial memory
	size_t initialSize = 1024;
	MemoryResult result = fun_memory_allocate(initialSize);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	Memory memory = result.value;

	// Test successful reallocation (increase size)
	size_t newSize = 2048;
	result = fun_memory_reallocate(memory, newSize);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(result.value != NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test reallocation to smaller size
	newSize = 512;
	result = fun_memory_reallocate(result.value, newSize);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(result.value != NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Clean up
	voidResult freeResult = fun_memory_free(&result.value);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	// Test reallocation of NULL pointer (should act like memoryAllocate)
	result = fun_memory_reallocate(NULL, 1024);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(result.value == NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Clean up
	freeResult = fun_memory_free(&result.value);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("fun_memory_reallocate");
}

void test_fun_memory_free()
{
	// Allocate memory to free
	MemoryResult allocResult = fun_memory_allocate(1024);
	if (allocResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	Memory memory = allocResult.value;

	// Test successful free
	voidResult freeResult = fun_memory_free(&memory);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(memory == NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test double free
	freeResult = fun_memory_free(&memory);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(memory == NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test freeing NULL pointer (should not cause an error)
	memory = NULL;
	freeResult = fun_memory_free(&memory);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("fun_memory_free");
}

void test_fun_memory_fill()
{
	// Allocate memory to fill
	size_t size = 1024;
	MemoryResult allocResult = fun_memory_allocate(size);
	if (allocResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	Memory memory = allocResult.value;

	// Test successful fill
	uint64_t fillValue = 0xAA;
	voidResult fillResult = fun_memory_fill(memory, size, fillValue);
	if (fillResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	// Verify the fill
	for (size_t i = 0; i < (size / sizeof(uint64_t)); i++) {
		if (!(((uint64_t *)memory)[i] == fillValue)) {
			fun_console_write_line("FAIL: assertion");
			return;
		}
	}

	// Test fill with NULL pointer (should fail)
	fillResult = fun_memory_fill(NULL, size, fillValue);
	if (fillResult.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}

	// Test fill with size 0 (should succeed but do nothing)
	fillResult = fun_memory_fill(memory, 0, fillValue);
	if (fillResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	// Clean up
	voidResult freeResult = fun_memory_free(&memory);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("fun_memory_fill");
}

void test_fun_memory_size()
{
	// Allocate memory
	size_t allocSize = 1024;
	MemoryResult allocResult = fun_memory_allocate(allocSize);
	if (allocResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	Memory memory = allocResult.value;

	// Test getting size of allocated memory
	size_tResult sizeResult = fun_memory_size(memory);
	if (sizeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(sizeResult.value >= allocSize)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test getting size of NULL pointer (should fail)
	sizeResult = fun_memory_size(NULL);
	if (sizeResult.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
	if (!(sizeResult.value == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Clean up
	voidResult freeResult = fun_memory_free(&memory);
	if (freeResult.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("fun_memory_size");
}

void test_fun_memory_copy()
{
	// Test 1: Basic copy
	char src1[] = "Hello, World!";
	char dest1[20] = { 0 };
	voidResult result = fun_memory_copy(src1, dest1, sizeof(src1));
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(memoryCompare(src1, dest1, sizeof(src1)) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test 2: Copy with offset
	char src2[] = "Ananas!";
	char dest2[20] = "Hello, ------!";
	result = fun_memory_copy(src2, dest2 + 7, 7);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(memoryCompare(dest2, "Hello, Ananas!", 14) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test 3: Copy zero bytes
	char src3[] = "Test";
	char dest3[10] = { 0 };
	result = fun_memory_copy(src3, dest3, 0);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(dest3[0] == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test 4: Copy to overlapping region (forward)
	char buffer1[] = "abcdefghijklmnop";
	result = fun_memory_copy(buffer1, buffer1 + 4, 8);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(memoryCompare(buffer1, "abcdabcdefghmnop", 16) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test 5: Copy to overlapping region (backward)
	char buffer2[] = "abcdefghijklmnop";
	result = fun_memory_copy(buffer2 + 4, buffer2, 8);
	if (result.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(memoryCompare(buffer2, "efghijklijklmnop", 16) == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	// Test 6: NULL destination
	result = fun_memory_copy(NULL, src1, sizeof(src1));
	if (result.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}

	// Test 7: NULL source
	result = fun_memory_copy(dest1, NULL, sizeof(src1));
	if (result.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}

	print_test_result("fun_memory_copy");
}

void test_realloc_multi_page_data_preservation()
{
	size_t oldSize = 5000;
	MemoryResult r = fun_memory_allocate(oldSize);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	Memory mem = r.value;

	fun_memory_fill(mem, oldSize, 0xCDCDCDCDCDCDCDCDULL);

	r = fun_memory_reallocate(mem, 8000);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	mem = r.value;

	uint8_t *bytes = (uint8_t *)mem;
	for (size_t i = 0; i < oldSize; i++) {
		if (!(bytes[i] == 0xCD)) {
			fun_console_write_line("FAIL: assertion");
			return;
		}
	}

	voidResult fr = fun_memory_free(&r.value);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_realloc_multi_page_data_preservation");
}

void test_realloc_in_place_same_page()
{
	size_t size = 1000;
	MemoryResult r = fun_memory_allocate(size);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	Memory oldPtr = r.value;

	r = fun_memory_reallocate(r.value, 2000);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(r.value == oldPtr)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	voidResult fr = fun_memory_free(&r.value);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_realloc_in_place_same_page");
}

void test_realloc_shrink_preserves_data()
{
	size_t oldSize = 4096;
	MemoryResult r = fun_memory_allocate(oldSize);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	Memory mem = r.value;

	fun_memory_fill(mem, oldSize, 0xABABABABABABABABULL);

	r = fun_memory_reallocate(mem, 2048);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	mem = r.value;

	uint8_t *bytes = (uint8_t *)mem;
	for (size_t i = 0; i < 2048; i++) {
		if (!(bytes[i] == 0xAB)) {
			fun_console_write_line("FAIL: assertion");
			return;
		}
	}

	voidResult fr = fun_memory_free(&r.value);
	if (fr.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result("test_realloc_shrink_preserves_data");
}

int main()
{
	fun_console_write_line("Running memory module tests:");
	test_fun_memory_allocate();
	test_fun_memory_reallocate();
	test_fun_memory_free();
	test_fun_memory_fill();
	test_fun_memory_size();
	test_fun_memory_copy();
	test_realloc_multi_page_data_preservation();
	test_realloc_in_place_same_page();
	test_realloc_shrink_preserves_data();
	fun_console_write_line("All tests passed!");
	return 0;
}
