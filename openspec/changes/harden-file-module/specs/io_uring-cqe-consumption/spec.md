# io_uring CQE Consumption

## Summary

Fix io_uring completion queue entry (CQE) handling to properly consume completions using `io_uring_cqe_seen()` and handle partial completions.

## Requirements

| ID | Requirement | Priority |
|----|-------------|----------|
| IU-01 | Track CQ head position in state | MUST |
| IU-02 | Use `io_uring_for_each_cqe` macro for iteration | MUST |
| IU-03 | Call `io_uring_cqe_seen` after processing | MUST |
| IU-04 | Handle partial read/write completions | MUST |
| IU-05 | Handle `IORING_CQE_F_MORE` flag | SHOULD |
| IU-06 | Handle CQE wraparound correctly | MUST |

## Implementation

### State Structure

```c
// include/fileReadRing.h
typedef struct {
    // Ring buffers
    struct io_uring ring;
    void *sq_ring;
    void *cq_ring;
    
    // CQ tracking
    unsigned cq_head;
    unsigned cq_tail;
    
    // Operation tracking
    uint64_t expected_user_data;
    uint64_t bytes_completed;
    uint64_t bytes_expected;
    
    // State flags
    bool submitted;
    bool completed;
} FileReadRingState;
```

### CQE Consumption

```c
// src/fileReadRing.c
AsyncResult fun_file_read_ring_poll(FileReadRingState *state) {
    if (!state || !state->submitted) {
        return ERROR_RESULT_INVALID_STATE;
    }
    
    if (state->completed) {
        return (AsyncResult){ .status = ASYNC_COMPLETED,
                              .error = ERROR_RESULT_NO_ERROR };
    }
    
    // Get CQ entries
    unsigned head = state->cq_head;
    struct io_uring_cqe *cqe;
    
    io_uring_for_each_cqe(&state->ring, head, cqe) {
        if (cqe->user_data != state->expected_user_data) {
            continue;  // Not our completion
        }
        
        // Check for error
        if (cqe->res < 0) {
            state->completed = true;
            state->cq_head = head + 1;
            io_uring_cqe_seen(&state->ring, cqe);
            return ERROR_RESULT_FROM_ERRNO(-cqe->res);
        }
        
        // Handle partial completion
        state->bytes_completed += cqe->res;
        
        if (state->bytes_completed >= state->bytes_expected) {
            state->completed = true;
        }
        
        // Mark CQE as seen
        state->cq_head = head + 1;
        io_uring_cqe_seen(&state->ring, cqe);
        break;
    }
    
    if (state->completed) {
        return (AsyncResult){ .status = ASYNC_COMPLETED,
                              .error = ERROR_RESULT_NO_ERROR };
    }
    
    return (AsyncResult){ .status = ASYNC_PENDING,
                          .error = ERROR_RESULT_NO_ERROR };
}
```

### Handling IORING_CQE_F_MORE

```c
// Check if more CQEs are coming for this operation
if (cqe->flags & IORING_CQE_F_MORE) {
    // Don't mark as completed yet - more data coming
    state->bytes_completed += cqe->res;
    io_uring_cqe_seen(&state->ring, cqe);
    return (AsyncResult){ .status = ASYNC_PENDING, ... };
}
```

### Submit and Wait

```c
AsyncResult fun_file_read_ring_submit(FileReadRingState *state) {
    int ret = io_uring_submit(&state->ring);
    if (ret < 0) {
        return ERROR_RESULT_FROM_ERRNO(-ret);
    }
    state->submitted = true;
    state->cq_head = 0;
    state->cq_tail = 0;
    state->bytes_completed = 0;
    
    return (AsyncResult){ .status = ASYNC_PENDING,
                          .error = ERROR_RESULT_NO_ERROR };
}
```

## Testing

```c
// tests/file_io_uring/test_file_io_uring.c
void test_partial_read_completion(void) {
    // Setup: Request 1MB read, io_uring returns 4KB chunks
    FileReadRingState state;
    memset(&state, 0, sizeof(state));
    
    // Submit read
    AsyncResult result = fun_file_read_ring_submit(&state);
    assert(result.status == ASYNC_PENDING);
    
    // Poll until completion
    int poll_count = 0;
    while (result.status == ASYNC_PENDING && poll_count < 300) {
        result = fun_file_read_ring_poll(&state);
        poll_count++;
        usleep(1000);
    }
    
    assert(result.status == ASYNC_COMPLETED);
    assert(state.bytes_completed == state.bytes_expected);
}

void test_cqe_wraparound(void) {
    // Submit many operations to wrap the CQ ring
    for (int i = 0; i < 1000; i++) {
        // Submit and complete
    }
    // Should not crash or lose CQEs
}
```

## Files Modified

- `include/fileReadRing.h`
- `include/fileWriteRing.h`
- `src/fileReadRing.c`
- `src/fileWriteRing.c`
- `tests/file_io_uring/test_file_io_uring.c` (new)
