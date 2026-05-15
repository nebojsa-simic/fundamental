## Context

The Fundamental Library follows a caller-allocated memory philosophy and uses opaque types. No synchronization primitives exist yet. The `thread-pool` module needs per-worker `Mutex` and `CondVar` for worker slot synchronization. Extracting these into a `sync` module avoids duplicating platform-specific code across modules.

## Goals / Non-Goals

**Goals:**
- `Mutex`: create, lock, unlock, destroy
- `CondVar`: create, wait (with associated mutex), signal, broadcast, destroy
- Caller-allocated opaque handles
- Cross-platform: Windows and Linux

**Non-Goals:**
- Recursive mutexes
- Timed waits (`pthread_cond_timedwait`)
- Read-write locks
- Semaphores
- Atomics
- Spin locks

## Decisions

### 1. `Mutex` and `CondVar` as separate opaque types

Each primitive has its own create/destroy lifecycle. `CondVar::wait` takes a `Mutex` parameter — the association is at the call site, not at creation. This matches POSIX semantics and keeps the API flat.

**Alternatives considered:**
- Combined `SyncPrimitive` with type tag — harder to validate at compile time
- CondVar owns its own mutex — less flexible; caller may want one mutex with multiple condvars

### 2. Caller-allocated opaque handles

`fun_mutex_create(&out_mutex)` allocates the internal platform struct and returns an opaque handle. `fun_mutex_destroy(mutex)` frees it. This follows the library's convention.

### 3. Platform implementations

Windows: `CRITICAL_SECTION` for mutex, `CONDITION_VARIABLE` for condvar.
Linux: `pthread_mutex_t` for mutex, `pthread_cond_t` for condvar.

### 4. No error return from lock/unlock/signal/broadcast

These operations cannot fail on either platform in normal use. They return `voidResult` — consistent with the library's convention for infallible operations.

**Alternatives considered:**
- `CanReturnError(void)` — adds ceremony with no benefit; lock failures indicate programmer error or memory corruption

## Risks / Trade-offs

| Risk | Mitigation |
|------|-----------|
| Lock/unlock mismatch (forgetting to unlock) | Caller's responsibility; consistent with library philosophy |
| Destroying mutex while held | Undefined — caller's responsibility |
| Signal before wait (lost wakeup) | Per POSIX, condvar signals are not queued; caller must use a predicate under the mutex |
