# Resource State Tracking

## Summary

Add explicit validity flags for each resource (file descriptor, mapped address, etc.) to prevent double-free and double-close errors in error paths.

## ADDED Requirements

### Requirement: Resource Validity Flags

Each resource SHALL have a `_valid` boolean flag to track its state.

#### Scenario: File descriptor validity
- **WHEN** file descriptor is allocated successfully
- **THEN** `file_descriptor_valid` is set to `true`
- **THEN** cleanup checks flag before calling `close()`

#### Scenario: Mapped address validity
- **WHEN** memory mapping is created successfully
- **THEN** `mapped_address_valid` is set to `true`
- **THEN** cleanup checks flag before calling `munmap()`

### Requirement: Cleanup Flag Checks

Cleanup functions SHALL check `_valid` flags before freeing resources.

#### Scenario: Check before close
- **WHEN** cleanup function is called
- **THEN** `file_descriptor_valid` is checked
- **THEN** `close()` is only called if flag is `true`

#### Scenario: Check before unmap
- **WHEN** cleanup function is called
- **THEN** `mapped_address_valid` is checked
- **THEN** `munmap()` is only called if flag is `true`

#### Scenario: No double-free
- **WHEN** cleanup is called twice
- **THEN** second call is a no-op (flags already `false`)
- **THEN** no crash or double-free occurs

### Requirement: Flag Set After Allocation

Validity flags SHALL be set immediately after successful allocation.

#### Scenario: Set after open
- **WHEN** `open()` syscall succeeds
- **THEN** `file_descriptor_valid = true` is set immediately
- **THEN** flag is set before any other operations

#### Scenario: Set after mmap
- **WHEN** `mmap()` syscall succeeds
- **THEN** `mapped_address_valid = true` is set immediately
- **THEN** flag is set before returning to caller

### Requirement: Flag Clear After Free

Validity flags SHALL be cleared after successful free/close.

#### Scenario: Clear after close
- **WHEN** `close()` syscall completes
- **THEN** `file_descriptor_valid = false` is set
- **THEN** file descriptor value is set to `-1`

#### Scenario: Clear after munmap
- **WHEN** `munmap()` syscall completes
- **THEN** `mapped_address_valid = false` is set
- **THEN** mapped address is set to `NULL`

### Requirement: Initialize Flags to False

All validity flags SHALL be initialized to `false` in state initialization.

#### Scenario: State zero-initialization
- **WHEN** state struct is created
- **THEN** all `_valid` flags are initialized to `false`
- **THEN** cleanup is safe to call even if allocation never happened

## Implementation

### State Structure

```c
// include/fileReadMmap.h
typedef struct {
    // File descriptor
    int file_descriptor;
    bool file_descriptor_valid;
    
    // Mapped memory
    void *mapped_address;
    bool mapped_address_valid;
    
    // Stat result
    struct stat file_stat;
    bool file_stat_valid;
    
    // Async state
    uint64_t adjusted_offset;
    uint64_t view_size;
    
    // Completion tracking
    bool cleanup_done;
} FileReadMmapState;
```

### Cleanup Function

```c
// src/fileReadMmap.c
static void file_read_mmap_cleanup(FileReadMmapState *state) {
    if (!state || state->cleanup_done) return;
    
    // Unmap if valid and not MAP_FAILED
    if (state->mapped_address_valid && 
        state->mapped_address != MAP_FAILED &&
        state->mapped_address != NULL) {
        syscall5(SYS_munmap, (long)state->mapped_address, state->view_size);
        state->mapped_address_valid = false;
        state->mapped_address = NULL;
    }
    
    // Close fd if valid and >= 0
    if (state->file_descriptor_valid && state->file_descriptor >= 0) {
        syscall1(SYS_close, state->file_descriptor);
        state->file_descriptor_valid = false;
        state->file_descriptor = -1;
    }
    
    state->cleanup_done = true;
}
```

### Usage Pattern

```c
AsyncResult fun_file_read_mmap(FileReadParameters params, 
                                FileReadMmapState *state) {
    // Zero-initialize (caller responsibility or use memset)
    // memset(state, 0, sizeof(*state));
    
    state->file_descriptor = -1;
    state->mapped_address = MAP_FAILED;
    
    // Open file
    int fd = (int)syscall2(SYS_open, (long)params.file_path, O_RDONLY);
    if (fd < 0) {
        return ERROR_RESULT_FROM_ERRNO(-fd);
    }
    state->file_descriptor = fd;
    state->file_descriptor_valid = true;  // Mark valid immediately
    
    // Stat file
    struct stat file_stat;
    long result = syscall2(SYS_fstat, fd, (long)&file_stat);
    if (result < 0) {
        goto cleanup;  // cleanup will close fd
    }
    state->file_stat = file_stat;
    state->file_stat_valid = true;
    
    // Map file
    void *addr = syscall6(SYS_mmap, 0, view_size, PROT_READ, 
                          MAP_SHARED, fd, state->adjusted_offset);
    if (addr == MAP_FAILED) {
        goto cleanup;  // cleanup will close fd
    }
    state->mapped_address = addr;
    state->mapped_address_valid = true;  // Mark valid immediately
    
    // ... rest of operation
    
cleanup:
    file_read_mmap_cleanup(state);
    return result;
}
```

## Testing

```c
// tests/file_resource_state/test_file_resource_state.c
void test_cleanup_after_open_failure(void) {
    // Mock open to succeed, fstat to fail
    FileReadMmapState state;
    memset(&state, 0, sizeof(state));
    
    AsyncResult result = mock_file_read_mmap(/* fstat fails */);
    assert(result.status == ASYNC_ERROR);
    assert(state.file_descriptor_valid == false);  // Cleaned up
    assert(state.file_descriptor == -1);
}

void test_cleanup_after_mmap_failure(void) {
    FileReadMmapState state;
    memset(&state, 0, sizeof(state));
    
    AsyncResult result = mock_file_read_mmap(/* mmap fails */);
    assert(result.status == ASYNC_ERROR);
    assert(state.file_descriptor_valid == false);  // Cleaned up
    assert(state.mapped_address_valid == false);
}

void test_no_double_free(void) {
    FileReadMmapState state;
    memset(&state, 0, sizeof(state));
    
    // Complete operation successfully
    AsyncResult result = mock_file_read_mmap(/* success */);
    assert(result.status == ASYNC_COMPLETED);
    
    // Call cleanup again - should be no-op
    file_read_mmap_cleanup(&state);
    // Should not crash or double-free
}
```

## Files Modified

- `include/fileReadMmap.h`
- `include/fileWriteMmap.h`
- `include/fileReadRing.h`
- `include/fileWriteRing.h`
- `include/fileAppend.h`
- `src/fileReadMmap.c`
- `src/fileWriteMmap.c`
- `src/fileReadRing.c`
- `src/fileWriteRing.c`
- `src/fileAppend.c`
- `tests/file_resource_state/test_file_resource_state.c` (new)
