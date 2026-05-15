## Why

The library has no synchronization primitives. Modules that need mutual exclusion or thread signaling (thread pool, future hashmap concurrency, async coordination) would duplicate platform-specific mutex and condition variable implementations. A minimal sync module provides `Mutex` and `CondVar` as opaque types with a uniform API across Windows and Linux.

## What Changes

- Add `Mutex` opaque type: `fun_mutex_create`, `fun_mutex_lock`, `fun_mutex_unlock`, `fun_mutex_destroy`
- Add `CondVar` opaque type: `fun_condvar_create`, `fun_condvar_wait`, `fun_condvar_signal`, `fun_condvar_broadcast`, `fun_condvar_destroy`
- Caller-allocated — creates return opaque handles via output parameters
- Platform-specific: Windows (`CRITICAL_SECTION` / `CONDITION_VARIABLE`) and Linux (`pthread_mutex_t` / `pthread_cond_t`)
- No dependency on any other module except `error` and `memory`

## Capabilities

### New Capabilities
- `sync`: Platform-abstracted `Mutex` and `CondVar` primitives. Caller-allocated opaque handles, consistent API across Windows and Linux.

### Modified Capabilities
- None

## Impact

- New header: `include/fundamental/sync/sync.h`
- New source files: `src/sync/`
- Platform-specific implementations: `arch/sync/windows-amd64/`, `arch/sync/linux-amd64/`
- Used by `thread-pool` module for per-worker slot synchronization
- No breaking changes
