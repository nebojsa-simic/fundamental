#include "../memory.h"
#include <windows.h>

CanReturnError(Memory) memoryAllocate(size_t size) {
    MemoryResult result;
    HANDLE hHeap = GetProcessHeap();
    result.Value = HeapAlloc(hHeap, 0, size);
    if (result.Value == NULL) {
        result.Error = errorResultCreate(GetLastError(), "Failed to allocate memory");
    } else {
        result.Error = errorResultCreate(0, NULL);
    }
    return result;
}

CanReturnError(Memory) memoryReallocate(Memory memory, size_t newSize) {
    MemoryResult result;
    HANDLE hHeap = GetProcessHeap();
    result.Value = HeapReAlloc(hHeap, 0, memory, newSize);
    if (result.Value == NULL) {
        result.Error = errorResultCreate(GetLastError(), "Failed to reallocate memory");
    } else {
        result.Error = errorResultCreate(0, NULL);
    }
    return result;
}

CanReturnError(void) memoryFree(Memory* memory) {
    voidResult result;
    HANDLE hHeap = GetProcessHeap();
    if (HeapFree(hHeap, 0, *memory)) {
        result.Error = errorResultCreate(0, NULL);
        *memory = NULL;
    } else {
        result.Error = errorResultCreate(GetLastError(), "Failed to free memory");
    }
    return result;
}

CanReturnError(void) memoryFill(Memory memory, size_t size, uint32_t value) {
    voidResult result;
    // Validate the memory pointer
    if (memory == NULL) {
        result.Error = errorResultCreate(ERROR_INVALID_PARAMETER, "Invalid memory pointer");
        return result;
    }
    HANDLE hHeap = GetProcessHeap();
    // Check if the memory block is valid using HeapValidate
    if (!HeapValidate(hHeap, 0, memory)) {
        result.Error = errorResultCreate(GetLastError(), "Invalid memory block");
        return result;
    }

    // Fill the memory
    for (size_t i = 0; i < size; i++) {
        ((uint8_t*)memory)[i] = (uint8_t)value;
    }
    
    result.Error = errorResultCreate(0, NULL);
    return result;
}

CanReturnError(size_t) memorySize(Memory memory) {
    size_tResult result;

    if (memory == NULL) {
        result.Value = 0;
        result.Error = errorResultCreate(ERROR_INVALID_PARAMETER, "Cannot get size of NULL pointer");
        return result;
    }
    
    HANDLE hHeap = GetProcessHeap();
    size_t size = HeapSize(hHeap, 0, memory);
    if (size == (size_t)-1) {
        result.Value = 0;
        result.Error = errorResultCreate(1, "Failed to get memory size");
    } else {
        result.Value = size;
        result.Error = errorResultCreate(0, NULL);
    }
    return result;
}

