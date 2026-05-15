## Context

The Fundamental Library has no thread management primitive. The async module provides `AsyncResult` for polling non-blocking operations but does not manage thread lifetimes. The network server module needs a way to dispatch per-connection work without blocking its accept thread. Applications need bounded concurrency for parallel file I/O, batch hashmap operations, and CPU-bound work.

The library follows a caller-allocated memory philosophy, uses opaque types, and returns error results via `CanReturnError()` macros. All modules work identically on Windows and Linux via platform-specific `arch/*/` implementations.

## Goals / Non-Goals

**Goals:**
- Fixed-size thread pool with configurable worker count, no queue — direct dispatch only
- `WorkItem` type as the unit of submission: caller populates, pool copies internally via `fun_memory_copy`
- Non-blocking submit: returns error when all workers are busy
- Pool owns and frees its internal copy after work function completes
- Per-worker synchronization via `Mutex` and `CondVar` from the `sync` module
- Cross-platform: Windows (`CreateThread`) and Linux (`pthread_create`)
- Drop-in use from any module's callback (network server, async completion, etc.)

**Non-Goals:**
- Dynamic thread count (grow/shrink based on load)
- Work prioritization or scheduling policies
- Per-work `AsyncResult` tracking (caller manages completion if needed)
- Thread-local storage or thread naming
- CPU affinity or NUMA awareness

## Decisions

### 1. Single create call — no separate config and start stages

The server module uses a two-stage API (config + listen) because configuration can fail validation before any thread is spawned. A thread pool has exactly two parameters: thread count and queue size. Neither can fail validation. A single `fun_thread_pool_create(num_threads, queue_size, &pool)` is simpler and avoids an intermediate opaque config type.

**Alternatives considered:**
- Two-stage (config + start) — adds API surface with no benefit; no validation step exists
- Config with setters — overengineered for two integer parameters

### 2. Non-blocking submit with data copy and `ERROR_CODE_THREAD_POOL_FULL`

`fun_thread_pool_submit(pool, const WorkItem *item)` copies `item->data` internally via `fun_memory_copy`. On success, the pool owns the copy and frees it after the worker completes. On failure (`ERROR_CODE_THREAD_POOL_FULL`), no copy is made and the caller retains full ownership. The caller always frees the original data after `submit()` returns — regardless of success or failure.

This is critical for the network server integration. If `submit()` blocked, the accept thread would stall, the OS backlog would fill, and new connections would be silently dropped at the TCP level. Non-blocking submit lets the callback close the connection immediately — the client gets a TCP RST rather than a timeout.

**Alternatives considered:**
- Blocking submit — simpler API but blocks caller; accept thread stalls; silent connection loss at OS level
- Callback-based rejection — complex; caller needs to provide both work function and rejection handler
- Separate `try_submit()` — adds function count; single function with error return is cleaner

### 3. Per-worker slots — no queue

Each worker has a slot containing the assigned `WorkItem` copy, a `Mutex`, and a `CondVar`. `submit()` iterates worker slots, locks each mutex, and checks if the slot is idle (`data == NULL`). If idle, it stores the pool-owned copy, signals the condvar, and returns OK. If all slots are busy, it frees the unneeded copy and returns `ERROR_CODE_THREAD_POOL_FULL`.

This eliminates the ring buffer entirely. No queue allocation, no enqueue/dequeue logic, no buffer sizing decisions. Backpressure is immediate: if all workers are busy, submit rejects instantly.

**Alternatives considered:**
- Bounded ring buffer queue — adds allocation, sizing decisions, and enqueue/dequeue logic; doubles the code size
- Unbounded queue — grows without limit; memory exhaustion under sustained load
- Per-worker queues with work stealing — adds complexity; not needed for expected workloads

### 4. `WorkItem` as the unit of submission with pool-owned copy

`WorkItem` is a single exposed type with three fields: `void *data`, `size_t data_size`, and `void (*work_fn)(void *)`. The caller populates all three, passes `const WorkItem *` to `submit()`, and the pool copies the data via `fun_memory_copy`. After `submit()` returns, the caller frees the original data. The pool owns and frees its copy after the worker completes.

This model eliminates ownership ambiguity. The return value of `submit()` signals only whether the copy was accepted, not who manages the memory. Caller always manages and frees the original. Pool always manages and frees the copy.

**Alternatives considered:**
- Caller passes raw `void *data` — pool doesn't know size, can't copy; ownership unclear on rejection
- Caller provides free function — adds parameter; violates "caller always frees original" simplicity
- Pool allocates data — breaks caller-allocated philosophy

### 5. Shutdown waits for in-progress work, joins workers

`fun_thread_pool_destroy(pool)` sets the stop flag, broadcasts all per-worker condvars to wake sleeping workers, waits for any in-progress work functions to complete, then joins all worker threads. After destroy returns, all workers have exited and resources are freed. There is no queue to drain — only in-progress work needs to finish.

**Alternatives considered:**
- Immediate shutdown (kill threads) — loses in-progress work; unsafe
- Timeout-based shutdown — adds parameter; caller can implement if needed

### 6. Synchronization via `sync` module

Each worker slot uses a `Mutex` and `CondVar` from a separate `sync` module. The sync module provides platform-abstracted synchronization primitives. This keeps the thread pool focused on dispatch logic and avoids duplicating mutex/condvar implementations across modules.

Per-worker slot pseudocode:

```
Worker:                               Submit:
  lock(slot.mutex)                      for each worker:
  while (slot.data == NULL)               lock(slot.mutex)
    if (stop) unlock, exit                  if (slot.data == NULL)
    wait(slot.cond, slot.mutex)               slot.data = copy
  data = slot.data                            signal(slot.cond)
  slot.data = NULL                            unlock(slot.mutex)
  unlock(slot.mutex)                          return OK
  data->work_fn(data)                       unlock(slot.mutex)
  fun_memory_free(data)                  fun_memory_free(copy)
  goto top                               return FULL
```

**Alternatives considered:**
- Single shared mutex + condvar — all workers contend on one lock; worse under load
- Lock-free CAS — workers would need to spin or use OS-specific wake mechanisms; condvar is standard

### 7. Platform-specific thread creation

Windows: `CreateThread()` with `_beginthreadex`-compatible worker function.
Linux: `pthread_create()` with default attributes.
Thread handles are stored in the pool struct and joined during destroy.

## Risks / Trade-offs

| Risk | Mitigation |
|------|-----------|
| Queue full under sustained load → work rejected | Caller controls queue size; non-blocking submit lets caller handle rejection gracefully |
| Worker crash (e.g., SIGSEGV in work function) | Process-level failure; not recoverable at pool level. Caller validates inputs before submit. |
| Destroy called while workers are running | Destroy drains queue and joins workers safely; caller must ensure no concurrent submits during destroy |
| Thread creation failure | `fun_thread_pool_create` returns error; caller retries or exits |
| Platform differences in thread scheduling | Library uses default OS scheduler; caller can adjust via OS-specific APIs if needed |
