## 1. Header and Module Structure

- [x] 1.1 Create `include/fundamental/thread_pool/thread_pool.h` with `ThreadPool` opaque type, `WorkItem` struct (`void *data`, `size_t data_size`, `void (*work_fn)(void *)`), and all function declarations
- [x] 1.2 Create `src/thread_pool/` directory and `thread_pool.c` with shared implementation (worker slot allocation, submit loop, worker loop stubs)
- [x] 1.3 Create `arch/thread_pool/windows-amd64/` directory with `thread_pool.c` for Windows platform
- [x] 1.4 Create `arch/thread_pool/linux-amd64/` directory with `thread_pool.c` for Linux platform
- [x] 1.5 Define `ERROR_CODE_THREAD_POOL_FULL` in the error module
- [x] 1.6 Depend on `sync` module for `Mutex` and `CondVar` per-worker slot synchronization

## 2. Internal Data Structures

- [x] 2.1 Define `WorkerSlot` struct (`WorkItem` copy, `Mutex`, `CondVar`) — per-worker state
- [x] 2.2 Define `ThreadPool` internal struct (array of `WorkerSlot`, worker thread handles, stop flag, thread count)

## 3. Pool Creation

- [x] 3.1 Implement `fun_thread_pool_create()` — validate `num_threads > 0`, allocate internal struct and `WorkerSlot` array
- [x] 3.2 Create per-worker `Mutex` and `CondVar` via sync module
- [x] 3.3 Implement worker thread spawning on Windows (`CreateThread`)
- [x] 3.4 Implement worker thread spawning on Linux (`clone` syscall, no pthreads)
- [x] 3.5 Handle thread creation failure: destroy any already-created threads and slots, free resources, return error

## 4. Work Submission

- [x] 4.1 Implement `fun_thread_pool_submit()` — validate `pool`, `item`, `item->data`, `item->data_size`, `item->work_fn` are not NULL/zero
- [x] 4.2 Allocate internal copy via `fun_memory_allocate(item->data_size)`
- [x] 4.3 Copy data via `fun_memory_copy(item->data, copy, item->data_size)`
- [x] 4.4 Iterate worker slots, lock mutex, check if slot is idle (`data == NULL`)
- [x] 4.5 If idle slot found: assign copy, signal condvar, unlock mutex, return OK
- [x] 4.6 If all slots busy: free internal copy, return `ERROR_CODE_THREAD_POOL_FULL`

## 5. Worker Loop

- [x] 5.1 Implement worker loop — lock slot mutex, wait on condvar while slot is idle and pool not stopped
- [x] 5.2 On wake with data: take ownership of `WorkItem` copy, set slot to idle, unlock mutex
- [x] 5.3 Invoke `work_fn(data)`, then free internal data copy via `fun_memory_free`
- [x] 5.4 Return to top of loop (slot is idle, next submit will find it)
- [x] 5.5 Handle stop signal: exit loop cleanly when stop flag is set

## 6. Pool Destroy

- [x] 6.1 Implement `fun_thread_pool_destroy()` — set stop flag, broadcast all per-worker condvars to wake workers
- [x] 6.2 Join all worker threads (Windows: `WaitForSingleObject`; Linux: `futex` on `clear_tid`)
- [x] 6.3 Destroy per-worker mutex and condvar via sync module, free `WorkerSlot` array and pool struct
- [x] 6.4 Handle NULL pool as no-op

## 7. Build Scripts

- [x] 7.1 Create `tests/thread-pool/build-windows-amd64.bat` with all source files
- [x] 7.2 Create `tests/thread-pool/build-linux-amd64.sh` with all source files (no `-lpthread`, uses `clone` syscall)

## 8. Tests

- [x] 8.1 Test pool creation with valid parameters (num_threads=4)
- [x] 8.2 Test pool creation with num_threads=0 returns error
- [x] 8.3 Test pool creation with NULL out_pool returns error
- [x] 8.4 Test submit succeeds, worker receives copied data (not caller's original pointer)
- [x] 8.5 Test caller can free original data after submit without affecting worker
- [x] 8.6 Test submit returns ERROR_CODE_THREAD_POOL_FULL when all workers busy
- [x] 8.7 Test submit returns ERROR_CODE_THREAD_POOL_FULL — no internal copy made, caller retains ownership
- [x] 8.8 Test submit with NULL pool returns error
- [x] 8.9 Test submit with NULL item returns error
- [x] 8.10 Test submit with NULL item->data returns error
- [x] 8.11 Test submit with NULL item->work_fn returns error
- [x] 8.12 Test destroy on NULL pool is no-op
- [x] 8.13 Test destroy waits for in-progress work to complete
- [x] 8.14 Test concurrent submits from multiple threads (no corruption, all work executed)

## 9. Validation

- [x] 9.1 Run `run-tests-windows-amd64.bat` — all thread-pool tests pass
- [ ] 9.2 Run `./run-tests-linux-amd64.sh` — all thread-pool tests pass (pending Linux)
- [x] 9.3 Run `code-format.bat` — clang-format passes on all new files
- [x] 9.4 Run `openspec validate thread-pool` — all specs validated
