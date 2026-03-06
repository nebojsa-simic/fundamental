# Collections Module Design

## Architecture Overview

The Collections module follows the same platform-agnostic architecture as other modules in the library, leveraging the cross-platform memory management layer instead of containing OS-specific code:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Collections Architecture                       │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  Public Interface                        Universal Implementation       │
│  ┌─────────────────┐    ┌─────────────────────────────────────────────┐ │
│  │ include/collection │    │ src/collections/                         │ │
│  │     array.h     │────│   ├── array.c                            │ │
│  │     map.h       │    │   ├── map.c                              │ │
│  │     tree.h      │    │   ├── tree.c                             │ │
│  │     set.h       │    │   └── utils.c                            │ │
│  └─────────────────┘    │                                            │ │
│                          └─────────────────────────────────────────────┘ │
│                                  │                                       │
│                                  ▼                                       │
│                  ┌─────────────────────────────────────────────┐         │
│                  │     Core Memory Management Layer          │         │
│                  │ (Cross-platform: arch/memory/*/memory.c)  │         │
│                  └─────────────────────────────────────────────┘         │
│                                  │                                       │
│   (Uses only fun_memory_* functions - no OS-specific calls)              │
└─────────────────────────────────────────────────────────────────────────┘
```

## Implementation Strategy

### Platform Independence Principle

The Collections module SHALL be completely platform-agnostic, interfacing only with the library's memory management functions:

- **No OS-specific code** in collections implementations
- **All memory operations** delegated to `fun_memory_` family functions  
- **Identical behavior** across all supported platforms
- **Same performance** and memory usage patterns on Windows, Linux, etc.

### Dynamic Array (Vector) Implementation

```
Array Internals:
┌─────────────────────────────────────────────────────────────────────────┐
│ typedef struct {                                                        │
│   void   *data;        ◄── Allocated memory block (via fun_memory)    │
│   size_t  count;       ◄── Current element count (used slots)          │
│   size_t  capacity;    ◄── Max capacity before realloc needed          │
│   size_t  element_size;◄── Size of each element in bytes               │
│ } Array;                                                                 │
└─────────────────────────────────────────────────────────────────────────┘

Memory layout: [elem0][elem1][elem2]...[elemN-1][unused...unused]
              │←── count=N ──→│    │←── capacity - N ──→│
```

#### Growth Strategy
- **Cross-platform**: Uses `fun_memory_reallocate` for all resizing needs
- **Capacity doubles** when full (2× growth factor) via memory abstraction  
- **Minimum capacity** to prevent excessive allocation for small arrays
- **Memory copied** using safe `fun_memory_copy` operations

### Hash Map Implementation

```
HashMap Structure:
┌─────────────────────────────────────────────┐
│ typedef struct {                            │
│   void     *buckets;    ◄── (via fun_memory_)                    │
│   size_t    bucket_count;   ◄── (managed via library memory)      │
│   size_t    element_count;  ◄── (library memory management)       │
│   void     *hash_fn;    (library-provided when platform-specific needed)  │
│   void     *compare_fn; (library-provided when platform-specific needed) │
│ } HashMap;                                   │
└─────────────────────────────────────────────┘
```

Hash maps shall rely entirely on the memory management layer for all allocations, making them platform-agnostic.

### Red-Black Tree Implementation

Self-balancing binary search tree using library-provided memory management:

```
RB-Tree Node:
┌─────────────────────────────────────────────┐
│ struct RBNode {                             │
│   void      *key;        ◄── (via fun_memory_)                  │
│   void      *value;      ◄── (via fun_memory_)                  │
│   struct RBNode *parent;  ◄── (library-managed memory)          │
│   struct RBNode *left;    ◄── (library-managed memory)          │
│   struct RBNode *right;   ◄── (library-managed memory)          │
│   int        color;      (pure logic - no platform difference)      │
│ }                                              │
└─────────────────────────────────────────────┘
```

## API Design 

Follows existing `fun_` naming convention with macro-expanded type safety, completely platform-agnostic:

```c
// Creation functions - use library memory functions exclusively
CanReturnError(IntArray) fun_array_int_create(size_t initial_capacity);
CanReturnError(StringIntHashMap) fun_hmap_string_int_create(size_t initial_bucket_count);

// Operations always use fun_memory_* functions internally
CanReturnError(void) fun_array_int_push(IntArray *arr, int value);
CanReturnError(int) fun_array_int_get(IntArray *arr, size_t index);
bool fun_array_int_has_data(IntArray *arr);

// Destruction exclusively via fun_memory_free
CanReturnError(void) fun_array_int_destroy(IntArray *arr);
```

## Type Safety Strategy

Use the macro approach for type safety with zero platform-specific code:

```c
#define DEFINE_ARRAY_TYPE(T) \
    typedef struct { \
        T *data;             /* Type-safe pointer via library memory */ \
        size_t count;        \
        size_t capacity;     \
    } T##Array;              /* E.g.: IntArray, StringArray */ \
    \
    CanReturnError(T##Array) fun_array_##T##_create(size_t cap); \
    CanReturnError(void) fun_array_##T##_push(T##Array *arr, T val); \
    CanReturnError(T) fun_array_##T##_get(T##Array *arr, size_t idx);

DEFINE_ARRAY_TYPE(int)
DEFINE_ARRAY_TYPE(double)
DEFINE_ARRAY_TYPE(void*)
```

## Error Handling

All functions follow the standardized `CanReturnError(*)` pattern and remain completely platform-agnostic:

- Memory allocation failures handled uniformly across platforms
- Index out of bounds returns error via standard error system
- Invalid parameters (NULL arrays, etc) return specific errors through library error handling
- No OS-specific error codes or logic in any collection code

## Memory Management

- **Universal pattern**: All collections rely on library's memory management only
- **Collection-owned**: Internal allocations use `fun_memory_allocate`/`fun_memory_free`
- **Safe operations**: All memory interactions through `fun_memory_copy`, etc.
- **No direct syscalls**: No direct calls to Windows VirtualAlloc, Linux mmap, etc.
- **OS-abstraction**: Collections remain identical across all supported platforms