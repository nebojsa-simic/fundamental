## ADDED Requirements

### Requirement: Pool Creation
The object pool module SHALL allow creation of a fixed-size, fixed-capacity object pool with a specified element size and capacity.

#### Scenario: Create pool with valid parameters
- **WHEN** fun_object_pool_create(elementSize, capacity) is called with elementSize >= sizeof(void*) and capacity > 0
- **THEN** a valid ObjectPool is returned
- **AND** the pool contains capacity free slots pre-wired into the free list
- **AND** all slots are immediately acquirable

#### Scenario: Create pool with element size too small
- **WHEN** fun_object_pool_create(elementSize, capacity) is called with elementSize < sizeof(void*)
- **THEN** an error is returned
- **AND** no memory is allocated

#### Scenario: Create pool with zero capacity
- **WHEN** fun_object_pool_create(elementSize, 0) is called
- **THEN** an error is returned

### Requirement: Slot Acquisition
The object pool SHALL provide O(1) slot acquisition from the free list without invoking any system call.

#### Scenario: Acquire from pool with free slots
- **WHEN** fun_object_pool_acquire(pool) is called and freeCount > 0
- **THEN** a valid slot pointer is returned
- **AND** freeCount is decremented by one

#### Scenario: Acquire from exhausted pool
- **WHEN** fun_object_pool_acquire(pool) is called and freeCount == 0
- **THEN** NULL is returned with an appropriate error

#### Scenario: Acquire from NULL pool
- **WHEN** fun_object_pool_acquire(NULL) is called
- **THEN** NULL is returned with an error

### Requirement: Slot Release
The object pool SHALL provide O(1) slot release back to the free list without invoking any system call.

#### Scenario: Release valid slot
- **WHEN** fun_object_pool_release(pool, slot) is called with a slot previously acquired from pool
- **THEN** the slot is returned to the free list
- **AND** freeCount is incremented by one

#### Scenario: Release slot not owned by pool
- **WHEN** fun_object_pool_release(pool, slot) is called with a slot not acquired from this pool
- **THEN** an error is returned
- **AND** the slot is NOT added to the free list

#### Scenario: Release NULL slot
- **WHEN** fun_object_pool_release(pool, NULL) is called
- **THEN** an error is returned

### Requirement: Pool Destruction
The object pool SHALL allow destruction of a pool, freeing the backing slab and detecting unreleased slots.

#### Scenario: Destroy fully released pool
- **WHEN** fun_object_pool_destroy(pool) is called and all slots have been released
- **THEN** the backing memory is freed via fun_memory_free
- **AND** no error is returned

#### Scenario: Destroy pool with outstanding slots
- **WHEN** fun_object_pool_destroy(pool) is called with freeCount < capacity
- **THEN** the backing memory is freed via fun_memory_free
- **AND** an error is returned indicating the number of leaked slots

#### Scenario: Destroy NULL pool
- **WHEN** fun_object_pool_destroy(NULL) is called
- **THEN** no operation is performed
- **AND** no error is returned

### Requirement: Pool Query
The object pool SHALL provide functions to query pool state.

#### Scenario: Query free count
- **WHEN** fun_object_pool_free_count(pool) is called
- **THEN** the number of currently free slots is returned

#### Scenario: Query total capacity
- **WHEN** fun_object_pool_capacity(pool) is called
- **THEN** the total number of slots in the pool is returned

#### Scenario: Query element size
- **WHEN** fun_object_pool_element_size(pool) is called
- **THEN** the element size the pool was created with is returned

### Requirement: Type-Safe Pool Macro
The object pool module SHALL provide a macro to generate type-safe pool wrappers following the existing collections macro pattern.

#### Scenario: Generate typed pool for custom struct
- **WHEN** DEFINE_OBJECT_POOL_TYPE(MyStruct) is invoked
- **THEN** a struct MyStructPool is defined containing an ObjectPool
- **AND** typed acquire, release, destroy, and query functions are generated

#### Scenario: Acquire and release via typed wrapper
- **WHEN** fun_object_pool_MyStruct_acquire(pool) is called
- **THEN** a MyStruct* is returned or NULL if exhausted
- **WHEN** fun_object_pool_MyStruct_release(pool, ptr) is called
- **THEN** the slot is returned to the pool

### Requirement: Demo Application
The object pool module SHALL include a demo application that demonstrates real-world usage.

#### Scenario: Demo builds and runs
- **WHEN** the demo is built with the platform build script
- **THEN** a working `lockd` executable is produced
- **AND** the lock service listens on TCP port 8080
- **AND** clients can `ACQUIRE <name>`, `RELEASE <name>`, `LIST`, and `QUIT`
- **AND** session pool exhaustion rejects new connections with `BUSY`
- **AND** lock pool exhaustion returns `NOLOCK`
- **AND** QUIT releases all locks held by that client
- **AND** console output reports connections, lock state changes, and pool stats

#### Scenario: Demo is cross-platform
- **WHEN** the demo is built on Linux AND Windows
- **THEN** it compiles and runs identically on both platforms
