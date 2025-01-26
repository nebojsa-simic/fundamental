#include "../memory.h"

// System call numbers
#define SYS_mmap 9
#define SYS_munmap 11
#define SYS_brk 12

// mmap flags
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_PRIVATE 0x2
#define MAP_ANONYMOUS 0x20

// Page size
#define PAGE_SIZE 4096

// Inline assembly for syscalls
static inline long syscall1(long n, long a1) {
    long ret;
    __asm__ __volatile__ (
        "syscall"
        : "=a" (ret)
        : "a" (n), "D" (a1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    register long r9 __asm__("r9") = a6;
    __asm__ __volatile__ (
        "syscall"
        : "=a" (ret)
        : "a" (n), "D" (a1), "S" (a2), "d" (a3), "r" (r10), "r" (r8), "r" (r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

CanReturnError(Memory) memoryAllocate(size_t size) {
    MemoryResult result;
    
    // If size is 0, allocate one page
    if (size == 0) {
        size = PAGE_SIZE;
    } else {
        // Round up to the nearest page size
        size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    }
    
    long ret = syscall6(SYS_mmap, 0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ret < 0 && ret > -4096) {
        result.Value = NULL;
        result.Error = errorResultCreate(-ret, "Failed to allocate memory");
    } else {
        result.Value = (void*)ret;
        result.Error = errorResultCreate(0, NULL);
    }
    return result;
}

CanReturnError(Memory) memoryReallocate(Memory memory, size_t newSize) {
    MemoryResult result;

    if (memory == NULL) {
        // If memory is NULL, return NULL without allocating
        result.Value = NULL;
        result.Error = errorResultCreate(0, NULL);
        return result;
    }

    // Allocate new memory
    result = memoryAllocate(newSize);
    if (result.Error.Code == 0) {
        // Copy old data
        size_t copySize = newSize < PAGE_SIZE ? newSize : PAGE_SIZE;
        for (size_t i = 0; i < copySize; i++) {
            ((char*)result.Value)[i] = ((char*)memory)[i];
        }
        
        // Free old memory
        voidResult freeResult = memoryFree(&memory);
        if (freeResult.Error.Code != 0) {
            // If free fails, we should still return the new allocation
            // but we might want to log this error somehow
        }
    }

    return result;
}

CanReturnError(void) memoryFree(Memory* memory) {
    voidResult result;
    if (*memory != NULL) {
        long ret = syscall1(SYS_brk, (long)*memory);
        if (ret < 0 && ret > -4096) {
            result.Error = errorResultCreate(-ret, "Failed to free memory");
        } else {
            *memory = NULL;
            result.Error = errorResultCreate(0, NULL);
        }
    } else {
        result.Error = errorResultCreate(0, NULL);
    }
    return result;
}

CanReturnError(void) memoryFill(Memory memory, size_t size, uint32_t value) {
    voidResult result;
    if (memory == NULL) {
        result.Error = errorResultCreate(22, "Invalid argument");
        return result;
    }
    for (size_t i = 0; i < size; i++) {
        ((unsigned char*)memory)[i] = (unsigned char)value;
    }
    result.Error = errorResultCreate(0, NULL);
    return result;
}

CanReturnError(size_t) memorySize(Memory memory) {
    size_tResult result;
    if (memory == NULL) {
        result.Value = 0;
        result.Error = errorResultCreate(22, "Invalid argument");
        return result;
    }
    // We can't easily determine the exact size of an allocated block
    // without additional bookkeeping. For simplicity, we'll return a
    // minimum size (e.g., 4096 bytes, typical page size).
    result.Value = 4096;
    result.Error = errorResultCreate(0, NULL);
    return result;
}
