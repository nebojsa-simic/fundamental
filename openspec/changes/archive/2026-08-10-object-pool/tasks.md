## 1. Module Structure

- [x] 1.1 Create `include/fundamental/object-pool/object-pool.h` — public API header with ObjectPool struct, function declarations, and DEFINE_OBJECT_POOL_TYPE macro
- [x] 1.2 Create `src/object-pool/object-pool.c` — implementation source file
- [x] 1.3 Create `tests/object-pool/` directory with `test.c`

## 2. Core Data Structures

- [x] 2.1 Define `ObjectPool` struct (elementSize, capacity, freeCount, freeList, memory)
- [x] 2.2 Define `DEFINE_OBJECT_POOL_TYPE(T)` macro for type-safe pool wrapper

## 3. Pool Lifecycle

- [x] 3.1 Implement `fun_object_pool_create(elementSize, capacity)` — validate inputs, allocate slab via fun_memory_allocate, build intrusive free list by iterating slots
- [x] 3.2 Implement `fun_object_pool_destroy(pool)` — detect leaked slots (freeCount < capacity), free slab via fun_memory_free

## 4. Slot Operations

- [x] 4.1 Implement `fun_object_pool_acquire(pool)` — pop head of free list, return NULL with error if exhausted
- [x] 4.2 Implement `fun_object_pool_release(pool, slot)` — validate slot ownership via address range check, push to free list

## 5. Query Functions

- [x] 5.1 Implement `fun_object_pool_free_count(pool)`
- [x] 5.2 Implement `fun_object_pool_capacity(pool)`
- [x] 5.3 Implement `fun_object_pool_element_size(pool)`

## 6. Build Scripts

- [x] 6.1 Create `tests/object-pool/build-linux-amd64.sh` — compile test.c + src/object-pool/object-pool.c + arch/memory/linux-amd64/memory.c
- [x] 6.2 Create `tests/object-pool/build-windows-amd64.bat` — compile test.c + src/object-pool/object-pool.c + arch/memory/windows-amd64/memory.c

## 7. Tests

- [x] 7.1 Test pool creation: valid params, too-small elementSize, zero capacity
- [x] 7.2 Test acquire/release: acquire all slots (verify distinct pointers), release all, acquire again (verify reuse)
- [x] 7.3 Test acquire from exhausted pool (returns NULL with error)
- [x] 7.4 Test release validation: release foreign slot (not owned by pool), release NULL
- [x] 7.5 Test destroy with outstanding slots (leak detection)
- [x] 7.6 Test destroy fully released pool (no error)
- [x] 7.7 Test query functions: freeCount, capacity, elementSize
- [x] 7.8 Test type-safe macro with a custom struct type (create typed pool, acquire, release, destroy)

## 8. Demo Application

- [x] 8.1 Create `demos/object-pool/demo.c` — single-node lock service: two typed object pools (`SessionPool` for per-client state, `LockPool` for named locks). Protocol: `ACQUIRE <name>` grabs a lock, `RELEASE <name>` frees it, `LIST` shows all locks and owners, `QUIT` releases all held locks and disconnects. Pool exhaustion → `BUSY` (session pool) or `NOLOCK` (lock pool). Auto-release on disconnect. Prints per-event console output
- [x] 8.2 Create `demos/object-pool/build-linux-amd64.sh` — compile demo.c + src/object-pool/object-pool.c + arch/memory/linux-amd64/memory.c + network (server/simple-async) + console + startup + async + string
- [x] 8.3 Create `demos/object-pool/build-windows-amd64.bat` — compile demo.c + src/object-pool/object-pool.c + arch/memory/windows-amd64/memory.c + network (server/simple-async) + console + startup + async + string

## 9. Validation

- [x] 9.1 Build and run tests on Linux (`./build-linux-amd64.sh && ./test`)
- [x] 9.2 Build and run tests on Windows (`build-windows-amd64.bat && test.exe`)
- [x] 9.3 Build and run demo on Linux
- [x] 9.4 Build and run demo on Windows
