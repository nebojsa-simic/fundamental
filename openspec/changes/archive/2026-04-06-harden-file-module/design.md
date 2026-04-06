# Harden File Module - Design

## Overview

This change adds robustness and correctness fixes to the file module. All changes maintain the existing architecture while adding defensive checks and proper resource management.

## Design Decisions

### 1. Integer Overflow Checks

All size calculations must check for overflow before use:

```c
// Before
uint64_t view_size = bytes_to_read + offset;

// After
if (__builtin_add_overflow(bytes_to_read, offset, &view_size)) {
    return ERROR_RESULT_INTEGER_OVERFLOW;
}
```

For portable C without builtins:
```c
static inline bool would_overflow_add(uint64_t a, uint64_t b) {
    return a > UINT64_MAX - b;
}
```

### 2. Resource State Tracking

Each resource gets an explicit validity flag:

```c
typedef struct {
    int file_descriptor;
    bool file_descriptor_valid;
    void *mapped_address;
    bool mapped_address_valid;
    // ... other resources
} FileReadMmapState;
```

Cleanup checks flags:
```c
if (state->mapped_address_valid && state->mapped_address != MAP_FAILED) {
    syscall5(SYS_munmap, (long)state->mapped_address, view_size);
}
if (state->file_descriptor_valid && state->file_descriptor >= 0) {
    syscall1(SYS_close, state->file_descriptor);
}
```

### 3. io_uring CQE Consumption

Properly consume CQEs using head tracking:

```c
// Before
struct io_uring_cqe *cqe = (struct io_uring_cqe *)state->cq_ring;
if (cqe->user_data == 1) { /* process */ }

// After
unsigned head = state->cq_head;
struct io_uring_cqe *cqe;
io_uring_for_each_cqe(&state->ring, head, cqe) {
    if (cqe->user_data == state->expected_user_data) {
        // Process and break
        state->cq_head = head + 1;
        break;
    }
}
```

### 4. Lock Timeout

Add timeout parameter with retry loop:

```c
// Before
AsyncResult fun_acquire_file_lock(String filePath, FileLockState *state);

// After
AsyncResult fun_acquire_file_lock(String filePath, FileLockState *state, 
                                   uint64_t timeout_ms);
```

Implementation:
```c
long result = syscall2(SYS_flock, fd, LOCK_EX | LOCK_NB);
if (result < 0 && errno == EWOULDBLOCK) {
    // Retry with exponential backoff until timeout
    uint64_t elapsed = 0;
    while (elapsed < timeout_ms) {
        usleep(1000);  // 1ms
        elapsed += 1;
        result = syscall2(SYS_flock, fd, LOCK_EX | LOCK_NB);
        if (result == 0) break;
    }
}
```

### 5. Notification Cleanup

Track all resources for proper cleanup:

```c
typedef struct {
    int inotify_fd;
    int watch_fd;
    bool inotify_fd_valid;
    bool watch_fd_valid;
} FileNotificationState;

AsyncResult fun_unregister_file_change_notification(FileNotificationState *state)
{
    if (!state) return ERROR_RESULT_NULL_POINTER;
    
    if (state->watch_fd_valid) {
        syscall2(SYS_inotify_rm_watch, state->inotify_fd, state->watch_fd);
        syscall1(SYS_close, state->watch_fd);
    }
    if (state->inotify_fd_valid) {
        syscall1(SYS_close, state->inotify_fd);
    }
    fun_memory_free(state);
    
    return SUCCESS_RESULT;
}
```

### 6. Buffer Validation

Validate before copy:

```c
if (state->parameters.bytes_to_read > state->parameters.output.capacity) {
    return ERROR_RESULT_BUFFER_TOO_SMALL;
}
```

### 7. Runtime Page Size

```c
// Before
#define PAGE_SIZE 4096

// After
static inline uint64_t get_page_size(void) {
    return (uint64_t)sysconf(_SC_PAGESIZE);
}
```

### 8. Durability Modes

```c
typedef enum {
    FILE_DURABILITY_ASYNC,   // Default - page cache only
    FILE_DURABILITY_SYNC,    // msync(MS_SYNC) after write
    FILE_DURABILITY_FULL     // fsync() after write
} FileDurabilityMode;

// In write operation:
if (state->parameters.durability == FILE_DURABILITY_SYNC) {
    syscall3(SYS_msync, (long)state->mapped_address, view_size, MS_SYNC);
} else if (state->parameters.durability == FILE_DURABILITY_FULL) {
    syscall1(SYS_fsync, state->file_descriptor);
}
```

### 9. Syscall Headers

```c
// Before
#define SYS_open 2
#define SYS_close 3

// After
#include <sys/syscall.h>
// Use SYS_open, SYS_close, etc. directly
```

## Testing Strategy

1. **Overflow tests:** Pass near-UINT64_MAX values to read/write operations
2. **Error path tests:** Force failures at each syscall, verify no leaks
3. **io_uring tests:** Verify CQE consumption under high load
4. **Lock timeout tests:** Verify timeout behavior with held locks
5. **Notification tests:** Verify cleanup with valgrind
6. **Portability tests:** Run on ARM/aarch64 to verify page size handling

## Migration Path

1. Add timeout parameter to lock calls: `fun_acquire_file_lock(path, &state, 5000)`
2. Add durability mode to writes: `state.parameters.durability = FILE_DURABILITY_SYNC`
