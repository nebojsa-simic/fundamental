# Collections Module Specification

## Purpose  
The Collections module provides modern dynamic data structures including arrays, maps, trees, and sets with type-safe operations, memory-efficient growth algorithms, and comprehensive error handling. This module expands fundamental library capabilities beyond basic operations to provide container data structures that scale efficiently with use.

### Core Components
- **Arrays** (Vectors): Homogeneous, dynamic arrays with automatic capacity management
- **Hash Maps**: Associative key-value storage with configurable hash functions  
- **Red-Black Trees**: Self-balancing ordered binary trees with guaranteed O(log n) operations
- **Sets**: Unique-element containers with fast membership operations

## Requirements

### Requirement: Array Core Operations
The collections module SHALL provide dynamic arrays supporting common vector operations.

#### Scenario: Create array 
- **WHEN** fun_array_int_create(capacity) is called with positive initial capacity
- **THEN** IntArray is returned with capacity allocated elements
- **AND** array.count equals 0 initially
- **IF** allocation fails
- **THEN** ErrorResult indicates allocation error

#### Scenario: Add elements to array
- **WHEN** fun_array_int_push(array, value) is called on non-full array
- **THEN** value is inserted at end of array
- **AND** array.count increases by one
- **AND** no reallocation occurs until capacity exceeded

#### Scenario: Handle array auto-reallocation
- **WHEN** fun_array_int_push causes capacity overflow
- **THEN** array doubles its capacity with min(INITIAL_CAPACITY) constraint
- **AND** existing elements preserved in memory-safe copy operation
- **IF** allocation of enlarged array fails
- **THEN** original array unchanged and error returned

#### Scenario: Get and set array elements  
- **WHEN** fun_array_int_get(array, index) is called with valid count 
- **THEN** element at specified index is safely returned
- **IF** index >= array.count
- **THEN** operation returns out-of-bounds error code

#### Scenario: Destroy array safely
- **WHEN** fun_array_int_destroy(array) is called with valid array
- **THEN** all array memory is properly freed using fun_memory_free
- **AND** array structure is reset to default state

### Requirement: Type Safety Through Macro Patterns
Collections module SHALL provide safe type operations using macro expansion patterns.

#### Scenario: Generate int-specific array operations
- **WHEN** DEFINE_ARRAY_TYPE(int) is used 
- **THEN** IntArray typedef is created
- **AND** fun_array_int_create, fun_array_int_push, fun_array_int_get functions available
- **AND** compile-time type safety enforced (passing DoubleArray to fun_array_int_push triggers error)

### Requirement: Memory Safety and Error Handling
Collections module SHALL maintain strict memory safety with error propagation throughout.

#### Scenario: Safe memory reallocation
- **WHEN** any collection operation requires growth and memory unavailable  
- **THEN** no partial operations occur on old memory
- **AND** original state preserved during allocation failure
- **AND** ErrorResult.code indicates specific memory allocation failure

#### Scenario: Prevent memory leaks
- **WHEN** fun_array_int_destroy is called on array
- **THEN** all internally-allocated memory is freed via library memory management
- **AND** caller does not free intermediate memory directly
- **IF** memory free operation has issues
- **THEN** ErrorResult propagates memory manager errors

### Requirement: Hash Map Associative Storage
The collections module SHALL provide high-performance key-value associative storage.

#### Scenario: Create hash map  
- **WHEN** fun_hmap_string_int_create(bucket_count) is called
- **THEN** HashMap with specified number of initial buckets is allocated
- **AND** hash function initialized for string keys
- **IF** allocation of buckets fails
- **THEN** ErrorResult.code indicates memory failure

#### Scenario: Insert key-value pairs
- **WHEN** fun_hmap_string_int_put(map, key, value) is called
- **THEN** key-value pair stored associatively in map structure
- **IF** key already exists
- **THEN** associated value is updated (not duplicated)
- **IF** memory for new entry unavailable
- **THEN** ErrorResult returned indicating allocation failure

#### Scenario: Retrieve values by key
- **WHEN** fun_hmap_string_int_get(map, key) is called for stored key
- **THEN** associated value is returned with success result
- **IF** key not present in map  
- **THEN** ErrorResult.code indicates KEY_NOT_FOUND condition

### Requirement: Performance Characteristics
Collections module SHALL provide predictable performance consistent with standard implementations.

#### Scenario: O(1) hash map access average case
- **WHEN** fun_hmap_string_int_get is called with uniformly distributed data
- **THEN** expected access performance is O(1) constant time
- **IF** significant hash collisions occur  
- **THEN** fallback mechanisms (RB-trees in overflow buckets) maintain reasonable performance

#### Scenario: O(log n) tree operations
- **WHEN** tree operations (insert, find, delete) are performed  
- **THEN** self-balancing ensures O(log n) performance guarantee
- **AND** red-black coloring invariant maintained during rotations

### Requirement: Standard Container Operations
Collections module SHALL provide standard container methods across all structures.

#### Scenario: Size queries
- **WHEN** fun_array_int_size, fun_hmap_string_int_size, etc. called 
- **THEN** current logical element count is returned (not memory capacity)
- **AND** value represents actual populated entries

#### Scenario: Empty structure handling
- **WHEN** operations performed on empty collections
- **THEN** appropriate boundary behaviors occur (get() fails, size() returns 0)
- **AND** no memory dereference errors occur

## Constraints
- All structures adhere to caller-allocated memory pattern for structures themselves
- Collections internally managed memory uses fun_memory_* functions exclusively
- Thread safety is not primary concern until async operations module extended
- Type safety prioritizes compile-time checking through macro systems
- Hash functions should maintain uniform distribution for common key types
- Memory growth algorithms should minimize fragmentation while maintaining performance