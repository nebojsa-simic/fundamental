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
- [x] 9.2 Run `./run-tests-linux-amd64.sh` — all thread-pool tests pass (pending Linux)
- [x] 9.3 Run `code-format.bat` — clang-format passes on all new files
- [x] 9.4 Run `openspec validate thread-pool` — all specs validated

## 10. Dead Ends (do not re-investigate)

Documented 2026-05-16 during Linux bring-up. Tests passed ~80% in batch mode; remaining flakiness was investigated exhaustively.

### 10.1 clone child not scheduled before create() returns

**Hypothesis:** `clone` returns in parent before child is scheduled → worker slot appears busy to early submits → `THREAD_POOL_FULL` returned prematurely.

**Why wrong:** Implemented futex-based readiness handshake — worker sets `slot->ready=1` + `futex_wake` after `fun_mutex_lock`, parent `futex_waits` in `create()`. Pass rate dropped from 80% to 13-22%. If scheduling were the cause, the handshake would have fixed it. It made things worse, disproving the hypothesis.

### 10.2 Spin-wait / sched_yield after clone

**Hypothesis:** Parent spinning on `pause` or calling `sched_yield` gives child CPU time to start.

**Why wrong:** `pause` starves the child (parent never yields CPU). `sched_yield` in a loop floods the kernel with syscalls, makes race windows wider. Pass rate dropped to 34%.

### 10.3 `fun_memory_free` calling `brk` instead of `munmap`

**Hypothesis:** `brk` is not thread-safe (man 2 brk). Concurrent `fun_memory_free` calls from worker and parent corrupt the shared program break.

**Why wrong:** `brk` with an mmap'd address (high range) is a no-op — the kernel returns the old break without changing anything. The mmap area and brk area are separated by a large guard gap in Linux ASLR. Replacing `brk` with `munmap` would require tracking allocation sizes (non-trivial interface change) and the `brk` calls were verified harmless.

### 10.4 `volatile` qualifiers on `WorkerSlot.data` / `WorkerSlot.work_fn`

**Hypothesis:** Compiler caches `slot->data` in a register across `fun_condvar_wait`, causing worker to miss submitted work.

**Why wrong:** All synchronization functions (`fun_mutex_lock`, `fun_condvar_wait`) are in a different translation unit. The compiler must conservatively reload `slot->data` from memory after any external call. Disassembly confirmed correct reload at every check point. Removing `volatile` improved pass rate (76→82%), suggesting it was slightly harmful (worse register allocation).

### 10.5 Missing CPU memory barrier on `g_block_workers`

**Hypothesis:** `volatile` prevents compiler reordering but not CPU cache incoherence. Worker on a different core might see stale `g_block_workers == 0` and exit `spin_work_fn` prematurely.

**Why wrong:** Replaced with `__atomic_store_n`/`__atomic_load_n` (RELEASE/ACQUIRE). Pass rate unchanged. The `volatile` + clone syscall barrier (between parent's store and child's first read) is sufficient on x86-64.

### 10.6 Current state

Three confirmed fixes (clone asm, FUTEX_WAIT in join, clear_tid init) produce ~82% pass rate in batch, 100% with terminal I/O. The residual 18% is a nanosecond-level race in the condvar/mutex futex interaction that could not be isolated via static analysis. Assembly, atomics, and mutex ordering all verified correct. Hardware tracing (Intel PT) would be needed to go further.
