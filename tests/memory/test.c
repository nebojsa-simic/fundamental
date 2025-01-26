#include <assert.h>
#include <stdio.h>
#include "../../src/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

// Helper function to print test progress
void print_test_result(const char* test_name) {
    printf("%s %s\n", GREEN_CHECK, test_name);
}

// Helper function to compare two memory regions
int memoryCompare(const void* s1, const void* s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

void test_memoryAllocate() {
    // Test successful allocation
    size_t size = 1024;
    MemoryResult result = memoryAllocate(size);
    ASSERT_NO_ERROR(result);
    assert(result.value != NULL);
    
    // Clean up
    voidResult freeResult = memoryFree(&result.value);
    ASSERT_NO_ERROR(freeResult);

    // Test allocation of size 0 (should succeed with minimum allocation)
    result = memoryAllocate(0);
    ASSERT_NO_ERROR(result);
    assert(result.value != NULL);

    // Clean up the zero-size allocation
    freeResult = memoryFree(&result.value);
    ASSERT_NO_ERROR(freeResult);

    // Test very large allocation (might fail depending on available memory)
    result = memoryAllocate((size_t)-1);
    ASSERT_ERROR(result);
    assert(result.value == NULL);

    print_test_result("memoryAllocate");
}

void test_memoryReallocate() {
    // Allocate initial memory
    size_t initialSize = 1024;
    MemoryResult result = memoryAllocate(initialSize);
    ASSERT_NO_ERROR(result);
    Memory memory = result.value;

    // Test successful reallocation (increase size)
    size_t newSize = 2048;
    result = memoryReallocate(memory, newSize);
    ASSERT_NO_ERROR(result);
    assert(result.value != NULL);

    // Test reallocation to smaller size
    newSize = 512;
    result = memoryReallocate(result.value, newSize);
    ASSERT_NO_ERROR(result);
    assert(result.value != NULL);

    // Clean up
    voidResult freeResult = memoryFree(&result.value);
    ASSERT_NO_ERROR(freeResult);

    // Test reallocation of NULL pointer (should act like memoryAllocate)
    result = memoryReallocate(NULL, 1024);
    ASSERT_NO_ERROR(result);
    assert(result.value == NULL);

    // Clean up
    freeResult = memoryFree(&result.value);
    ASSERT_NO_ERROR(freeResult);

    print_test_result("memoryReallocate");
}

void test_memoryFree() {
    // Allocate memory to free
    MemoryResult allocResult = memoryAllocate(1024);
    ASSERT_NO_ERROR(allocResult);
    Memory memory = allocResult.value;

    // Test successful free
    voidResult freeResult = memoryFree(&memory);
    ASSERT_NO_ERROR(freeResult);
    assert(memory == NULL);

    // Test double free
    freeResult = memoryFree(&memory);
    ASSERT_NO_ERROR(freeResult);
    assert(memory == NULL);

    // Test freeing NULL pointer (should not cause an error)
    memory = NULL;
    freeResult = memoryFree(&memory);
    ASSERT_NO_ERROR(freeResult);

    print_test_result("memoryFree");
}

void test_memoryFill() {
    // Allocate memory to fill
    size_t size = 1024;
    MemoryResult allocResult = memoryAllocate(size);
    ASSERT_NO_ERROR(allocResult);
    Memory memory = allocResult.value;

    // Test successful fill
    uint64_t fillValue = 0xAA;
    voidResult fillResult = memoryFill(memory, size, fillValue);
    ASSERT_NO_ERROR(fillResult);

    // Verify the fill
    for (size_t i = 0; i < (size / sizeof(uint64_t)); i++) {
        assert(((uint64_t*)memory)[i] == fillValue);
    }

    // Test fill with NULL pointer (should fail)
    fillResult = memoryFill(NULL, size, fillValue);
    ASSERT_ERROR(fillResult);

    // Test fill with size 0 (should succeed but do nothing)
    fillResult = memoryFill(memory, 0, fillValue);
    ASSERT_NO_ERROR(fillResult);

    // Clean up
    voidResult freeResult = memoryFree(&memory);
    ASSERT_NO_ERROR(freeResult);

    print_test_result("memoryFill");
}

void test_memorySize() {
    // Allocate memory
    size_t allocSize = 1024;
    MemoryResult allocResult = memoryAllocate(allocSize);
    ASSERT_NO_ERROR(allocResult);
    Memory memory = allocResult.value;

    // Test getting size of allocated memory
    size_tResult sizeResult = memorySize(memory);
    ASSERT_NO_ERROR(sizeResult);
    assert(sizeResult.value >= allocSize);  // May be larger due to alignment

    // Test getting size of NULL pointer (should fail)
    sizeResult = memorySize(NULL);
    ASSERT_ERROR(sizeResult);
    assert(sizeResult.value == 0);

    // Clean up
    voidResult freeResult = memoryFree(&memory);
    ASSERT_NO_ERROR(freeResult);

    print_test_result("memorySize");
}


void test_memoryCopy() {
    // Test 1: Basic copy
    char src1[] = "Hello, World!";
    char dest1[20] = {0};
    voidResult result = memoryCopy(src1, dest1, sizeof(src1));
    ASSERT_NO_ERROR(result);
    assert(memoryCompare(src1, dest1, sizeof(src1)) == 0);

    // Test 2: Copy with offset
    char src2[] = "Ananas!";
    char dest2[20] = "Hello, ------!";
    result = memoryCopy(src2, dest2 + 7, 7);
    ASSERT_NO_ERROR(result);
    assert(memoryCompare(dest2, "Hello, Ananas!", 14) == 0);

    // Test 3: Copy zero bytes
    char src3[] = "Test";
    char dest3[10] = {0};
    result = memoryCopy(src3, dest3, 0);
    ASSERT_NO_ERROR(result);
    assert(dest3[0] == 0);

    // Test 4: Copy to overlapping region (forward)
    char buffer1[] = "abcdefghijklmnop";
    result = memoryCopy(buffer1, buffer1 + 4, 8);
    ASSERT_NO_ERROR(result);
    assert(memoryCompare(buffer1, "abcdabcdefghmnop", 16) == 0);

    // Test 5: Copy to overlapping region (backward)
    char buffer2[] = "abcdefghijklmnop";
    result = memoryCopy(buffer2 + 4, buffer2, 8);
    ASSERT_NO_ERROR(result);
    assert(memoryCompare(buffer2, "efghijklijklmnop", 16) == 0);

    // Test 6: NULL destination
    result = memoryCopy(NULL, src1, sizeof(src1));
    ASSERT_ERROR(result);

    // Test 7: NULL source
    result = memoryCopy(dest1, NULL, sizeof(src1));
    ASSERT_ERROR(result);

    print_test_result("memoryCopy");
}

int main() {
    printf("Running memory module tests:\n");
    test_memoryAllocate();
    test_memoryReallocate();
    test_memoryFree();
    test_memoryFill();
    test_memorySize();
    test_memoryCopy();
    printf("All tests passed!\n");
    return 0;
}
