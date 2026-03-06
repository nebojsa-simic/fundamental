# Collections Module Design

## Architecture Overview

The Collections module follows the same kernel-inspired, platform-agnostic architecture as other modules in the library:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Collections Architecture                       │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  Public Interface                    Platform-Specific Implementations │
│  ┌─────────────────┐    ┌─────────────────────────────────────────────┐ │
│  │ include/collection │    │ arch/collections/                         │ │
│  │     array.h     │────│   ├── windows-amd64/                       │ │
│  │     map.h       │    │   │   ├── array.c                          │ │
│  │     tree.h      │    │   │   ├── map.c                            │ │
│  │     set.h       │    │   │   └── tree.c                           │ │
│  └─────────────────┘    │   └── linux-amd64/                         │ │
│                          │       ├── array.c                          │ │
│  Core Implementation     │       ├── map.c                            │ │
│  ┌─────────────────┐    │       └── tree.c                           │ │
│  │ src/collections │    └─────────────────────────────────────────────┘ │
│  │     common.h    │                                                     │
│  │     common.c    │                                                     │
│  │   type_traits.h │                                                     │
│  └─────────────────┘                                                     │
└─────────────────────────────────────────────────────────────────────────┘
```

## Implementation Strategy

### Dynamic Array (Vector) Implementation

```
Array Internals:
┌─────────────────────────────────────────────────────────────────────────┐
│ typedef struct {                                                        │
│   void   *data;        ◄─── Allocated memory block                    │
│   size_t  count;       ◄─── Current element count (used slots)        │
│   size_t  capacity;    ◄─── Max capacity before realloc needed        │
│   size_t  element_size;◄─── Size of each element in bytes             │
│ } Array;                                                                 │
└─────────────────────────────────────────────────────────────────────────┘

Memory layout: [elem0][elem1][elem2]...[elemN-1][unused...unused]
              │←── count=N ──→│    │←── capacity - N ──→│
```

#### Growth Strategy
- Capacity doubles when full (2× growth factor)
- Minimum capacity to prevent excessive allocation for small arrays
- Memory copied using safe `fun_memory_copy` operations

### Hash Map Implementation

```
HashMap Structure:
┌─────────────────────────────────────────────┐
│ typedef struct {                            │
│   void     *buckets;    ◄── Array of hash buckets   │
│   size_t    bucket_count;   ◄── Number of buckets      │
│   size_t    element_count;  ◄── Total stored pairs     │
│   void     *hash_fn;    ◄── Hash function pointer    │
│   void     *compare_fn; ◄── Element comparison function│
│ } HashMap;                                   │
└─────────────────────────────────────────────┘

Bucket Layout:
[ [kv1│→│kv1│→│overflow_list] [empty] [kv3│→│kv3] [kv2] ... ]
│    \0x5F                  │      │ \0xA2        │ \0x7C     │
│    hash→bucket_idx 5      │      │ hash→bucket_idx  7       │
│                           │      └ bucket_idx 7 has single pair
└ bucket_idx 5 collides, uses linked list
```

For worst-case collision handling, we implement the kernel-inspired RB-tree approach within buckets where chain lengths exceed threshold (typically 8 elements).

### Red-Black Tree Implementation

Self-balancing binary search tree using kernel-style RED/BLACK coloring nodes:

```
RB-Tree Node:
┌─────────────────────────────────────────────┐
│ struct RBNode {                             │
│   void      *key;        ◄── Key data          │
│   void      *value;      ◄── Value data (for Map)│
│   struct RBNode *parent;  ◄── Parent reference   │
│   struct RBNode *left;    ◄── Left child         │
│   struct RBNode *right;   ◄── Right child        │
│   int        color;      ◄── RB_COLOR_RED/_BLACK │
│ }                                              │
└─────────────────────────────────────────────┘
```

## API Design 

Follows existing `fun_` naming convention with macro-expanded type safety:

```c
// Creation functions
CanReturnError(IntArray) fun_array_int_create(size_t initial_capacity);
CanReturnError(StringIntHashMap) fun_hmap_string_int_create(size_t initial_bucket_count);

// Common operations 
CanReturnError(void) fun_array_int_push(IntArray *arr, int value);
CanReturnError(int) fun_array_int_get(IntArray *arr, size_t index);
bool fun_array_int_has_data(IntArray *arr);

// Destruction
CanReturnError(void) fun_array_int_destroy(IntArray *arr);
```

## Type Safety Strategy

Use the macro approach for type safety instead of void*:

```c
#define DEFINE_ARRAY_TYPE(T) \
    typedef struct { \
        T *data;             /* Type-safe pointer */ \
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

All functions follow the standardized `CanReturnError(*)` pattern:

- Memory allocation failures return error codes
- Index out of bounds returns error rather than crashing
- Hash collisions are resolved gracefully
- Invalid parameters (NULL arrays, etc) return specific errors

## Memory Management

- Caller-owned pattern for root structures themselves
- Collection-owned for internal allocated elements
- Proper cleanup ensures no memory leakages
- Memory freed using `fun_memory_free` consistent with library principle