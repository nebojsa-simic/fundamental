# Collections Module Delta Changes

## Why

Add a modern set of collections to the fundamental library providing arrays, maps, trees and sets - OS-agnostic data structures that rely on the existing library's cross-platform memory management system.

## ADDED Requirements

### Requirement: Array Core Operations
The collections module SHALL provide dynamic arrays supporting common vector operations using the library's cross-platform memory functions for all allocation needs.

#### Scenario: Create array 
- **WHEN** fun_array_int_create(capacity) is called with positive initial capacity
- **THEN** IntArray is returned with capacity allocated elements
- **AND** array.count equals 0 initially
- **IF** allocation fails
- **THEN** ErrorResult indicates allocation error
- **AND** function internally uses fun_memory_allocate for memory acquisition

#### Scenario: Add elements to array
- **WHEN** fun_array_int_push(array, value) is called on non-full array
- **THEN** value is inserted at end of array
- **AND** array.count increases by one
- **AND** no reallocation occurs until capacity exceeded
- **IF** resize needed
- **THEN** function internally uses fun_memory_reallocate to grow storage

#### Scenario: Handle array auto-reallocation
- **WHEN** fun_array_int_push causes capacity overflow
- **THEN** array doubles its capacity with min(INITIAL_CAPACITY) constraint
- **AND** existing elements preserved in memory-safe copy operation
- **IF** allocation of enlarged array fails
- **THEN** original array unchanged and error returned
- **AND** reallocation uses fun_memory_reallocate internally

#### Scenario: Get and set array elements  
- **WHEN** fun_array_int_get(array, index) is called with valid count 
- **THEN** element at specified index is safely returned
- **IF** index >= array.count
- **THEN** operation returns out-of-bounds error code

#### Scenario: Destroy array safely
- **WHEN** fun_array_int_destroy(array) is called with valid array
- **THEN** all array memory is properly freed using fun_memory_free
- **AND** array structure is reset to default state
- **AND** all internal element memory released through library's memory management

### Requirement: Type Safety Through Macro Patterns
Collections module SHALL provide safe type operations using macro expansion patterns that work across all platforms.

#### Scenario: Generate int-specific array operations
- **WHEN** DEFINE_ARRAY_TYPE(int) is used 
- **THEN** IntArray typedef is created
- **AND** fun_array_int_create, fun_array_int_push, fun_array_int_get functions available
- **AND** compile-time type safety enforced across all platforms
- **AND** platform-agnostic type handling without OS-specific code

### Requirement: Memory Safety and Error Handling
Collections module SHALL remain OS-agnostic and rely on the fundamental library's memory management for all allocation needs.

#### Scenario: Safe memory reallocation
- **WHEN** any collection operation requires growth and memory unavailable  
- **THEN** no partial operations occur on old memory
- **AND** original state preserved during allocation failure
- **AND** ErrorResult.code indicates specific memory allocation failure
- **AND** all memory operations delegated to fun_memory_* functions

#### Scenario: Prevent memory leaks
- **WHEN** fun_array_int_destroy is called on array
- **THEN** all internally-allocated memory is freed via library memory management
- **AND** caller does not free intermediate memory directly
- **IF** memory free operation has issues
- **THEN** ErrorResult propagates memory manager errors
- **AND** no OS-specific memory code present in collection implementations

### Requirement: Hash Map Associative Storage
The collections module SHALL provide OS-agnostic hash map functionality using library's cross-platform memory services for all allocation needs.

#### Scenario: Create hash map  
- **WHEN** fun_hmap_string_int_create(bucket_count) is called
- **THEN** HashMap with specified number of initial buckets is allocated
- **AND** hash function initialized for string keys
- **IF** allocation of buckets fails
- **THEN** ErrorResult.code indicates memory failure
- **AND** all allocation handled through fun_memory_ functions internally

#### Scenario: Insert key-value pairs
- **WHEN** fun_hmap_string_int_put(map, key, value) is called
- **THEN** key-value pair stored associatively in map structure
- **IF** key already exists
- **THEN** associated value is updated (not duplicated)
- **IF** memory for new entry unavailable
- **THEN** ErrorResult returned indicating allocation failure
- **AND** internal key/value storage memory managed through library's fun_memory_* functions

#### Scenario: Retrieve values by key
- **WHEN** fun_hmap_string_int_get(map, key) is called for stored key
- **THEN** associated value is returned with success result
- **IF** key not present in map  
- **THEN** ErrorResult.code indicates KEY_NOT_FOUND condition
- **AND** no platform-specific code used for value retrieval

### Requirement: Performance Characteristics
Collections module SHALL provide OS-agnostic performance with predictable characteristics consistent across platforms.

#### Scenario: O(1) hash map access average case
- **WHEN** fun_hmap_string_int_get is called with uniformly distributed data
- **THEN** expected access performance is O(1) constant time
- **IF** significant hash collisions occur  
- **THEN** fallback mechanisms maintain reasonable performance
- **AND** performance identical across Windows, Linux and other supported platforms

#### Scenario: O(log n) tree operations
- **WHEN** tree operations (insert, find, delete) are performed  
- **THEN** self-balancing ensures O(log n) performance guarantee
- **AND** red-black coloring invariant maintained during rotations
- **AND** tree operations remain platform-agnostic using library functions only

### Requirement: Standard Container Operations
Collections module SHALL provide standard container methods across all structures using OS-agnostic operations and memory management from the library.

#### Scenario: Size queries
- **WHEN** fun_array_int_size, fun_hmap_string_int_size, etc. called 
- **THEN** current logical element count is returned (not memory capacity)
- **AND** value represents actual populated entries
- **AND** operation has no OS-specific code

#### Scenario: Empty structure handling
- **WHEN** operations performed on empty collections
- **THEN** appropriate boundary behaviors occur (get() fails, size() returns 0)
- **AND** no memory dereference errors occur
- **AND** all implementations rely purely on library functions and types

### Requirement: Platform Independence
Collections module SHALL not contain any OS-specific code and operate identically on all supported platforms.

#### Scenario: Platform-agnostic operation
- **WHEN** any collection function is executed on any supported platform
- **THEN** function behavior is identical across all platforms
- **AND** results and memory layouts are consistent
- **IF** OS-specific memory management is required
- **THEN** it is handled through fun_memory_* functions only

#### Scenario: Cross-platform allocation
- **WHEN** collection requires memory allocation or deallocation
- **THEN** functions only use fun_memory_* family of functions
- **AND** no direct syscalls or OS memory functions used in collection code
- **AND** Windows VirtualAlloc, Linux mmap, etc. handled in architecture-specific layer

### Requirement: Boundary Condition Handling
Collections module SHALL handle extreme memory and size conditions gracefully while remaining OS-agnostic.

#### Scenario: Maximum memory stress in arrays
- **WHEN** fun_array_int_push repeatedly called until system memory exhaustion
- **THEN** all existing elements preserved in stable state
- **AND** ErrorResult.code indicates OUT_OF_MEMORY condition
- **AND** partial element additions prevent silent data loss
- **AND** all memory stress handling occurs through library functions

#### Scenario: Zero capacity initial array
- **WHEN** fun_array_int_create(0) called with zero initial capacity
- **THEN** IntArray returned with initial growth-ready state
- **AND** first push operation triggers minimal allocation
- **IF** minimal growth allocation fails
- **THEN** error propagated as EXPECTED_CAPACITY_ZERO condition
- **AND** all allocations performed through fun_memory_allocate

#### Scenario: Empty array access
- **WHEN** fun_array_int_get(array, 0) called on empty array (count=0)
- **THEN** no memory dereference occurs for invalid index
- **AND** ErrorResult.code indicates INDEX_OUT_OF_BOUNDS
- **AND** array data structure state remains unchanged

#### Scenario: Massive collision scenarios
- **WHEN** fun_hmap_string_int_put called with multiple keys producing same hash
- **THEN** collision handled via chained storage mechanism
- **AND** retrieval performance degrades gracefully to O(n) for that specific bucket
- **IF** bucket overflow occurs for performance
- **THEN** fallback mechanism activated (platform-agnostic)
- **AND** all memory management for overflow uses only fun_memory_* functions

#### Scenario: High growth rate handling
- **WHEN** array repeatedly grows from 1 → 2 → 4 → 8 → ... → large sizes 
- **THEN** memory utilization maintains 50% efficiency during growing phase
- **AND** no intermediate memory allocations leaked during resize
- **IF** system memory cannot accommodate requested growth
- **THEN** previous array state preserved and error returned without corruption
- **AND** all resizing operations via fun_memory_reallocate

#### Scenario: Duplicate key handling in maps
- **WHEN** fun_hmap_string_int_put called with existing key
- **THEN** previous value is properly freed to prevent memory leak
- **AND** new value replaces the old value in associative storage
- **AND** map count remains stable (unchanged count)
- **AND** memory deallocation for old values uses fun_memory_free

#### Scenario: Invalid parameter guard
- **WHEN** fun_array_int_get called with NULL array pointer
- **THEN** no segmentation fault occurs from null dereference
- **AND** ErrorResult.code indicates INVALID_PARAMETER_NULL_PTR condition
- **AND** function fails gracefully with error rather than crashing
- **AND** error handling remains platform-agnostic

#### Scenario: Underflow handling in collections
- **WHEN** fun_array_int_remove_last called on empty array
- **THEN** count remains at 0 without decrementing past lower bound
- **AND** ErrorResult indicates COLLECTION_EMPTY_CONDITION
- **AND** array internal state remains consistent

#### Scenario: Memory fragmentation mitigation
- **WHEN** long-running collection operations create allocation/deallocation cycles
- **THEN** internal memory management attempts optimization via efficient growth strategies
- **AND** memory efficiency maintained during sustained operation
- **IF** defragmentation needed
- **THEN** handled internally by optimized reallocation pattern
- **AND** all memory manipulations using safe library functions

### Requirement: Resource Cleanup Completeness
Collections module SHALL ensure complete release of all allocated memory during destruction using only library functions.

#### Scenario: OS-agnostic cleanup
- **WHEN** collections contain elements requiring cleanup
- **THEN** destructors properly clean up through library-provided mechanisms
- **AND** no memory resource abandoned by final destructor call
- **AND** all memory release operations use fun_memory_free exclusively
- **AND** no platform-specific cleanup code present

#### Scenario: Interrupted destruction safety 
- **WHEN** fun_array_int_destroy interrupted during multi-stage cleanup
- **THEN** partial cleanup state prevents resource double-release via library safeguards
- **AND** remaining resources properly tracked for external recovery
- **OR** ErrorResult indicates PARTIAL_CLEANUP_OCCURRED status
- **AND** all cleanup operations remain within library's memory management system

## Constraints
- Collections contain ZERO OS-specific code and run identically on all supported platforms
- All memory allocation and deallocation performed only through fun_memory_* function family  
- Collections do not directly interface with OS system call interfaces
- Thread safety considerations deferred to async module extensions only
- Type safety priorities remain consistent with compile-time checking through macro systems
- Hash functions maintain uniform behavior across all supported platforms
- All memory growth algorithms maintain cross-platform consistency through library abstraction
- Error codes follow standardized library patterns with consistent naming across modules