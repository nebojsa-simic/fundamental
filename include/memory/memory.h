#ifndef LIBRARY_MEMORY_H
#define LIBRARY_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "../error/error.h"

// Define the memory module related types
typedef void *Memory;
// Define the corresponding error type
DEFINE_RESULT_TYPE(Memory);

// Interface
CanReturnError(Memory) fun_memory_allocate(size_t size);
CanReturnError(Memory) fun_memory_reallocate(Memory memory, size_t newSize);
CanReturnError(void) fun_memory_free(Memory *memory);
CanReturnError(void)
	fun_memory_fill(Memory memory, size_t size, uint64_t value);
CanReturnError(size_t) fun_memory_size(Memory memory);
CanReturnError(void)
	fun_memory_copy(const Memory source, const Memory destination,
					size_t sizeInBytes);

// ============================================================================
// Low-Level Memory Operations (for arch code, no error handling)
// These work with raw pointers for performance-critical code
// ============================================================================

// Fill memory with byte value (replaces memset)
static inline void fun_memset_bytes(void *ptr, uint8_t value, size_t size)
{
	uint8_t *p = (uint8_t *)ptr;
	for (size_t i = 0; i < size; i++) {
		p[i] = value;
	}
}

// Copy memory bytes (replaces memcpy)
static inline void fun_memcpy_bytes(void *dest, const void *src, size_t size)
{
	uint8_t *d = (uint8_t *)dest;
	const uint8_t *s = (const uint8_t *)src;
	for (size_t i = 0; i < size; i++) {
		d[i] = s[i];
	}
}

#endif // LIBRARY_MEMORY_H
