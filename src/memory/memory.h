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
CanReturnError(Memory) memoryAllocate(size_t size);
CanReturnError(Memory) memoryReallocate(Memory memory, size_t newSize);
CanReturnError(void) memoryFree(Memory *memory);
CanReturnError(void) memoryFill(Memory memory, size_t size, uint64_t value);
CanReturnError(size_t) memorySize(Memory memory);
CanReturnError(void) memoryCopy(const Memory source, const Memory destination,
				size_t sizeInBytes);

#endif // LIBRARY_MEMORY_H
