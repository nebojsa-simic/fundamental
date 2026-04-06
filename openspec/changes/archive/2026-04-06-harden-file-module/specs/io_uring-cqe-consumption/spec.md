# io_uring CQE Consumption

## Summary

Fix io_uring completion queue entry (CQE) handling to properly consume completions using `io_uring_cqe_seen()` and handle partial completions.

## ADDED Requirements

### Requirement: CQ Head Position Tracking

The state SHALL track the completion queue head position.

#### Scenario: Track cq_head
- **WHEN** io_uring operation is submitted
- **THEN** `cq_head` is stored in state struct
- **THEN** `cq_head` is used with `io_uring_for_each_cqe` macro

### Requirement: CQE Iteration Macro

CQE iteration SHALL use `io_uring_for_each_cqe` macro.

#### Scenario: Iterate CQEs
- **WHEN** polling for completions
- **THEN** `io_uring_for_each_cqe(&ring, head, cqe)` is used
- **THEN** iteration starts from stored `cq_head` position

### Requirement: Mark CQE as Seen

After processing each CQE, `io_uring_cqe_seen` SHALL be called.

#### Scenario: Mark processed CQE
- **WHEN** CQE is processed
- **THEN** `io_uring_cqe_seen(&ring, cqe)` is called
- **THEN** CQ head is advanced

### Requirement: Partial Completion Handling

Partial read/write completions SHALL be handled correctly.

#### Scenario: Partial read
- **WHEN** `cqe->res < expected_bytes`
- **THEN** `bytes_processed` is incremented by `cqe->res`
- **THEN** remaining I/O is submitted if needed

#### Scenario: Track bytes completed
- **WHEN** multiple CQEs arrive
- **THEN** `bytes_completed` accumulates all `cqe->res` values
- **THEN** operation completes when `bytes_completed >= bytes_expected`

### Requirement: IORING_CQE_F_MORE Flag

The `IORING_CQE_F_MORE` flag SHALL be handled.

#### Scenario: MORE flag set
- **WHEN** `cqe->flags & IORING_CQE_F_MORE` is true
- **THEN** more CQEs are expected for this operation
- **THEN** operation does not complete until MORE is cleared

### Requirement: CQE Wraparound

CQE wraparound SHALL be handled correctly.

#### Scenario: Ring buffer wrap
- **WHEN** CQ tail reaches ring size
- **THEN** wraparound to index 0 is handled
- **THEN** `io_uring_for_each_cqe` macro handles wrap correctly

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
