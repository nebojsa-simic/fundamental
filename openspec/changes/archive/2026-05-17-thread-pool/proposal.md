## Why

The library has no general-purpose mechanism for dispatching work across multiple threads. The network server module accepts connections synchronously in a callback — if the callback blocks, the accept thread stalls. CLI applications that need bounded concurrency (server callbacks, parallel file operations, batch processing) currently must implement their own thread management. A minimal thread pool provides a single primitive that every module can use.

## What Changes

- Add `ThreadPool` opaque type created via `fun_thread_pool_create(num_threads, &pool)`
- One call creates and starts the pool — no separate config stage
- Direct dispatch: work is handed to a worker immediately or rejected — no queue, no buffering
- Add `fun_thread_pool_submit(pool, const WorkItem *item)` returning `CanReturnError(void)` — copies `item->data` internally via `fun_memory_copy`, returns `ERROR_CODE_THREAD_POOL_FULL` when all workers are busy, without blocking
- Add `fun_thread_pool_destroy(pool)` — signals stop, waits for all workers to finish in-progress work, joins worker threads
- Worker threads: `void (*)(void *)` work function signature, receives pool-owned copy of submitted data
- Cross-platform: Windows `CreateThread` / Linux `pthread_create`
- Caller-allocated memory throughout; pool frees its internal copy after work function completes
- Depends on `sync` module for `Mutex` and `CondVar` per-worker slot synchronization

## Capabilities

### New Capabilities
- `thread-pool`: Fixed-size direct-dispatch thread pool with non-blocking submit. `WorkItem` submitted by caller, data copied by pool via `fun_memory_copy`. Pool owns and frees its copy after execution. No queue — workers are dispatched directly or submit returns `ERROR_CODE_THREAD_POOL_FULL`. Per-worker `Mutex` and `CondVar` from the `sync` module for slot synchronization.

### Modified Capabilities
- None

## Impact

- New header: `include/fundamental/thread_pool/thread_pool.h`
- New source files: `src/thread_pool/`
- Platform-specific implementations: `arch/thread_pool/windows-amd64/`, `arch/thread_pool/linux-amd64/`
- New test directory: `tests/thread-pool/`
- Used by `network-server` demo for non-blocking connection dispatch
- No breaking changes to existing modules
