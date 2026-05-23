## 1. Module Structure

- [ ] 1.1 Create `include/fundamental/object-pool/object-pool.h` — public API header with ObjectPool struct, function declarations, and DEFINE_OBJECT_POOL_TYPE macro
- [ ] 1.2 Create `src/object-pool/object-pool.c` — implementation source file
- [ ] 1.3 Create `tests/object-pool/` directory with `test.c`

## 2. Core Data Structures

- [ ] 2.1 Define `ObjectPool` struct (elementSize, capacity, freeCount, freeList, memory)
- [ ] 2.2 Define `DEFINE_OBJECT_POOL_TYPE(T)` macro for type-safe pool wrapper

## 3. Pool Lifecycle

- [ ] 3.1 Implement `fun_object_pool_create(elementSize, capacity)` — validate inputs, allocate slab via fun_memory_allocate, build intrusive free list by iterating slots
- [ ] 3.2 Implement `fun_object_pool_destroy(pool)` — detect leaked slots (freeCount < capacity), free slab via fun_memory_free

## 4. Slot Operations

- [ ] 4.1 Implement `fun_object_pool_acquire(pool)` — pop head of free list, return NULL with error if exhausted
- [ ] 4.2 Implement `fun_object_pool_release(pool, slot)` — validate slot ownership via address range check, push to free list

## 5. Query Functions

- [ ] 5.1 Implement `fun_object_pool_free_count(pool)`
- [ ] 5.2 Implement `fun_object_pool_capacity(pool)`
- [ ] 5.3 Implement `fun_object_pool_element_size(pool)`

## 6. Build Scripts

- [ ] 6.1 Create `tests/object-pool/build-linux-amd64.sh` — compile test.c + src/object-pool/object-pool.c + arch/memory/linux-amd64/memory.c
- [ ] 6.2 Create `tests/object-pool/build-windows-amd64.bat` — compile test.c + src/object-pool/object-pool.c + arch/memory/windows-amd64/memory.c

## 7. Tests

- [ ] 7.1 Test pool creation: valid params, too-small elementSize, zero capacity
- [ ] 7.2 Test acquire/release: acquire all slots (verify distinct pointers), release all, acquire again (verify reuse)
- [ ] 7.3 Test acquire from exhausted pool (returns NULL with error)
- [ ] 7.4 Test release validation: release foreign slot (not owned by pool), release NULL
- [ ] 7.5 Test destroy with outstanding slots (leak detection)
- [ ] 7.6 Test destroy fully released pool (no error)
- [ ] 7.7 Test query functions: freeCount, capacity, elementSize
- [ ] 7.8 Test type-safe macro with a custom struct type (create typed pool, acquire, release, destroy)

## 8. Demo Application

- [ ] 8.1 Create `demos/object-pool/demo.c` — single-node lock service: two typed object pools (`SessionPool` for per-client state, `LockPool` for named locks). Protocol: `ACQUIRE <name>` grabs a lock, `RELEASE <name>` frees it, `LIST` shows all locks and owners, `QUIT` releases all held locks and disconnects. Pool exhaustion → `BUSY` (session pool) or `NOLOCK` (lock pool). Auto-release on disconnect. Prints per-event console output
- [ ] 8.2 Create `demos/object-pool/build-linux-amd64.sh` — compile demo.c + src/object-pool/object-pool.c + arch/memory/linux-amd64/memory.c + network (server/simple-async) + console + startup + async + string
- [ ] 8.3 Create `demos/object-pool/build-windows-amd64.bat` — compile demo.c + src/object-pool/object-pool.c + arch/memory/windows-amd64/memory.c + network (server/simple-async) + console + startup + async + string

## 9. Validation

- [ ] 9.1 Build and run tests on Linux (`./build-linux-amd64.sh && ./test`)
- [ ] 9.2 Build and run tests on Windows (`build-windows-amd64.bat && test.exe`)
- [ ] 9.3 Build and run demo on Linux
- [ ] 9.4 Build and run demo on Windows
