#include <assert.h>
#include <stdio.h>
#include "../../src/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.Error.Code == 0)

// Helper function to print test progress
void print_test_result(const char* test_name) {
    printf("%s %s\n", GREEN_CHECK, test_name);
}

void test_memoryAllocate() {
    // Test successful allocation
    size_t size = 1024;
    MemoryResult result = memoryAllocate(size);
    ASSERT_NO_ERROR(result);
    assert(result.Value != NULL);
    
    // Clean up
    voidResult freeResult = memoryFree(&result.Value);
    ASSERT_NO_ERROR(freeResult);

    // Test allocation of size 0 (should succeed with minimum allocation)
    result = memoryAllocate(0);
    ASSERT_NO_ERROR(result);
    assert(result.Value != NULL);

    // Clean up the zero-size allocation
    freeResult = memoryFree(&result.Value);
    ASSERT_NO_ERROR(freeResult);

    // Test very large allocation (might fail depending on available memory)
    result = memoryAllocate((size_t)-1);
    assert(result.Error.Code != 0);
    assert(result.Value == NULL);

    print_test_result("memoryAllocate");
}

void test_memoryReallocate() {
    // Allocate initial memory
    size_t initialSize = 1024;
    MemoryResult result = memoryAllocate(initialSize);
    ASSERT_NO_ERROR(result);
    Memory memory = result.Value;

    // Test successful reallocation (increase size)
    size_t newSize = 2048;
    result = memoryReallocate(memory, newSize);
    ASSERT_NO_ERROR(result);
    assert(result.Value != NULL);

    // Test reallocation to smaller size
    newSize = 512;
    result = memoryReallocate(result.Value, newSize);
    ASSERT_NO_ERROR(result);
    assert(result.Value != NULL);

    // Clean up
    voidResult freeResult = memoryFree(&result.Value);
    ASSERT_NO_ERROR(freeResult);

    // Test reallocation of NULL pointer (should act like memoryAllocate)
    result = memoryReallocate(NULL, 1024);
    ASSERT_NO_ERROR(result);
    assert(result.Value == NULL);

    // Clean up
    freeResult = memoryFree(&result.Value);
    ASSERT_NO_ERROR(freeResult);

    print_test_result("memoryReallocate");
}

void test_memoryFree() {
    // Allocate memory to free
    MemoryResult allocResult = memoryAllocate(1024);
    ASSERT_NO_ERROR(allocResult);
    Memory memory = allocResult.Value;

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
    Memory memory = allocResult.Value;

    // Test successful fill
    uint32_t fillValue = 0xAA;
    voidResult fillResult = memoryFill(memory, size, fillValue);
    ASSERT_NO_ERROR(fillResult);

    // Verify the fill
    for (size_t i = 0; i < size; i++) {
        assert(((uint8_t*)memory)[i] == (uint8_t)fillValue);
    }

    // Test fill with NULL pointer (should fail)
    fillResult = memoryFill(NULL, size, fillValue);
    assert(fillResult.Error.Code != 0);

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
    Memory memory = allocResult.Value;

    // Test getting size of allocated memory
    size_tResult sizeResult = memorySize(memory);
    ASSERT_NO_ERROR(sizeResult);
    assert(sizeResult.Value >= allocSize);  // May be larger due to alignment

    // Test getting size of NULL pointer (should fail)
    sizeResult = memorySize(NULL);
    assert(sizeResult.Error.Code != 0);
    assert(sizeResult.Value == 0);

    // Clean up
    voidResult freeResult = memoryFree(&memory);
    ASSERT_NO_ERROR(freeResult);

    print_test_result("memorySize");
}

int main() {
    printf("Running memory module tests:\n");
    test_memoryAllocate();
    test_memoryReallocate();
    test_memoryFree();
    test_memoryFill();
    test_memorySize();
    printf("All tests passed!\n");
    return 0;
}
