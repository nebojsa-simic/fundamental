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

    // Calculate how many 64-bit chunks we can fill
    size_t chunkCount = size / sizeof(uint64_t);
    // Calculate leftover bytes if size isn't a multiple of 8
    size_t remainder = size % sizeof(uint64_t);

    // Fill 64-bit chunks
    uint64_t* ptr64 = (uint64_t*)memory;
    for (size_t i = 0; i < chunkCount; i++) {
        ptr64[i] = value;
    }

    // Fill remainder bytes
    if (remainder > 0) {
        uint8_t* remainderPtr = (uint8_t*)(ptr64 + chunkCount);
        // We copy the first 'remainder' bytes from 'value' to the leftover area
        const uint8_t* valueBytes = (const uint8_t*)&value;
        for (size_t j = 0; j < remainder; j++) {
            remainderPtr[j] = valueBytes[j];
        }
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

