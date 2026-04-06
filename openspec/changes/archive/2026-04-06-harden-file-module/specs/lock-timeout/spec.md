# Lock Timeout

## Summary

Add configurable timeout to file lock acquisition to prevent indefinite blocking and deadlocks.

## ADDED Requirements

### Requirement: Timeout Parameter

Lock operations SHALL accept a `timeout_ms` parameter.

#### Scenario: Custom timeout
- **WHEN** `fun_acquire_file_lock` is called with `timeout_ms`
- **THEN** lock acquisition waits up to `timeout_ms` milliseconds
- **THEN** returns error if lock not acquired within timeout

#### Scenario: Default timeout
- **WHEN** `fun_acquire_file_lock` is called without explicit timeout
- **THEN** default timeout of 5000ms is used
- **THEN** behavior is consistent with explicit timeout

### Requirement: Non-Blocking Lock with Retry

Lock acquisition SHALL use non-blocking lock with retry loop.

#### Scenario: Non-blocking attempt
- **WHEN** attempting to acquire lock
- **THEN** first attempt uses `LOCK_EX | LOCK_NB`
- **THEN** returns immediately if lock unavailable

#### Scenario: Retry loop
- **WHEN** non-blocking lock fails with EAGAIN/EWOULDBLOCK
- **THEN** retry after waiting `LOCK_RETRY_INTERVAL_MS`
- **THEN** continue until timeout expires or lock acquired

### Requirement: Timeout Error

Timeout SHALL return `ERROR_RESULT_LOCK_TIMEOUT`.

#### Scenario: Timeout expires
- **WHEN** timeout_ms elapses without acquiring lock
- **THEN** async result status is `ASYNC_ERROR`
- **THEN** error code is `ERROR_RESULT_LOCK_TIMEOUT`

### Requirement: Monotonic Time Tracking

Timeout SHALL use monotonic time source.

#### Scenario: Monotonic clock
- **WHEN** tracking elapsed time
- **THEN** `clock_gettime(CLOCK_MONOTONIC)` is used
- **THEN** system clock changes do not affect timeout

### Requirement: Error Code Definition

The error code `ERROR_RESULT_LOCK_TIMEOUT` SHALL be defined.

#### Scenario: Lock timeout error
- **WHEN** lock acquisition times out
- **THEN** error code `ERROR_RESULT_LOCK_TIMEOUT` is returned
- **THEN** error code value is 15 (as defined in error.h)

## Implementation

### API Change

```c
// Before
AsyncResult fun_acquire_file_lock(String filePath, FileLockState *state);

// After
AsyncResult fun_acquire_file_lock(String filePath, FileLockState *state,
                                   uint64_t timeout_ms);
```

### State Structure

```c
// include/fileLock.h
typedef struct {
    int file_descriptor;
    bool file_descriptor_valid;
    bool locked;
    uint64_t timeout_ms;
} FileLockState;
```

### Implementation

```c
// src/fileLock.c
#include <errno.h>
#include <time.h>

AsyncResult fun_acquire_file_lock(String filePath, FileLockState *state,
                                   uint64_t timeout_ms)
{
    if (!filePath || !state) {
        return ERROR_RESULT_NULL_POINTER;
    }
    
    memset(state, 0, sizeof(*state));
    state->file_descriptor = -1;
    state->timeout_ms = timeout_ms;
    
    // Open file
    int fd = (int)syscall3(SYS_open, (long)filePath, O_RDWR, 0644);
    if (fd < 0) {
        return ERROR_RESULT_FROM_ERRNO(-fd);
    }
    state->file_descriptor = fd;
    state->file_descriptor_valid = true;
    
    // Try non-blocking lock first
    long result = syscall2(SYS_flock, fd, LOCK_EX | LOCK_NB);
    if (result == 0) {
        state->locked = true;
        return (AsyncResult){ .status = ASYNC_COMPLETED,
                              .error = ERROR_RESULT_NO_ERROR };
    }
    
    if (errno != EWOULDBLOCK && errno != EAGAIN) {
        // Real error, not contention
        return ERROR_RESULT_FROM_ERRNO(errno);
    }
    
    // Lock is held - retry with timeout
    uint64_t elapsed_ms = 0;
    uint64_t backoff_us = 1000;  // Start at 1ms
    const uint64_t max_backoff_us = 100000;  // Max 100ms
    
    while (elapsed_ms < timeout_ms) {
        // Sleep with backoff
        usleep(backoff_us);
        elapsed_ms += backoff_us / 1000;
        
        // Exponential backoff
        backoff_us *= 2;
        if (backoff_us > max_backoff_us) {
            backoff_us = max_backoff_us;
        }
        
        // Try lock again
        result = syscall2(SYS_flock, fd, LOCK_EX | LOCK_NB);
        if (result == 0) {
            state->locked = true;
            return (AsyncResult){ .status = ASYNC_COMPLETED,
                                  .error = ERROR_RESULT_NO_ERROR };
        }
        
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            return ERROR_RESULT_FROM_ERRNO(errno);
        }
    }
    
    // Timeout
    return ERROR_RESULT_LOCK_TIMEOUT;
}
```

### Error Code

```c
// include/file_error.h
#define ERROR_RESULT_LOCK_TIMEOUT ((uint32_t)0x000000XX)
```

### Release Lock (Unchanged)

```c
AsyncResult fun_release_file_lock(FileLockState *state)
{
    if (!state || !state->file_descriptor_valid) {
        return ERROR_RESULT_INVALID_STATE;
    }
    
    if (state->locked) {
        syscall2(SYS_flock, state->file_descriptor, LOCK_UN);
        state->locked = false;
    }
    
    if (state->file_descriptor >= 0) {
        syscall1(SYS_close, state->file_descriptor);
        state->file_descriptor_valid = false;
    }
    
    return (AsyncResult){ .status = ASYNC_COMPLETED,
                          .error = ERROR_RESULT_NO_ERROR };
}
```

## Testing

```c
// tests/file_lock/test_file_lock.c
void test_lock_timeout(void) {
    // Create file and lock it
    FileLockState holder;
    AsyncResult result = fun_acquire_file_lock("/tmp/locktest", &holder, 5000);
    assert(result.status == ASYNC_COMPLETED);
    
    // Try to acquire with short timeout - should fail
    FileLockState waiter;
    result = fun_acquire_file_lock("/tmp/locktest", &waiter, 100);
    assert(result.status == ASYNC_ERROR);
    assert(result.error.code == ERROR_RESULT_LOCK_TIMEOUT);
    
    // Release
    fun_release_file_lock(&holder);
}

void test_lock_success_with_contention(void) {
    // Create file and lock it
    FileLockState holder;
    fun_acquire_file_lock("/tmp/locktest", &holder, 5000);
    
    // Start thread to release after 50ms
    // Try to acquire with 500ms timeout - should succeed after release
    FileLockState waiter;
    result = fun_acquire_file_lock("/tmp/locktest", &waiter, 500);
    assert(result.status == ASYNC_COMPLETED);
    
    fun_release_file_lock(&holder);
    fun_release_file_lock(&waiter);
}

void test_default_timeout(void) {
    // Verify reasonable default behavior
    FileLockState state;
    AsyncResult result = fun_acquire_file_lock("/tmp/nonexistent", &state, 5000);
    // Should fail quickly (file doesn't exist), not hang
    assert(result.status == ASYNC_ERROR);
}
```

## Files Modified

- `include/fileLock.h`
- `src/fileLock.c`
- `include/file_error.h`
- `tests/file_lock/test_file_lock.c`
