# thread-pool Specification

## Purpose
TBD - created by archiving change thread-pool. Update Purpose after archive.
## Requirements
### Requirement: Pool can be created with thread count
The system SHALL provide `fun_thread_pool_create(num_threads, &out_pool)` that creates a `ThreadPool` with the specified number of worker threads. On success, `*out_pool` is set to a valid pool handle and all workers are running. On error, `*out_pool` is unchanged. `num_threads` MUST be greater than 0. The pool SHALL use direct dispatch — workers receive work immediately or submit returns `ERROR_CODE_THREAD_POOL_FULL`.

#### Scenario: Valid parameter creates pool successfully
- **WHEN** `fun_thread_pool_create` is called with `num_threads = 4`
- **THEN** the result SHALL be OK, `*out_pool` SHALL be a valid handle, and 4 worker threads SHALL be running

#### Scenario: Zero threads returns error
- **WHEN** `fun_thread_pool_create` is called with `num_threads = 0`
- **THEN** the function SHALL return an error result and `*out_pool` SHALL be unchanged

#### Scenario: NULL output pointer returns error
- **WHEN** `fun_thread_pool_create` is called with a NULL `out_pool`
- **THEN** the function SHALL return an error result

### Requirement: Work can be submitted to the pool
The system SHALL provide `fun_thread_pool_submit(pool, const WorkItem *item)` that copies the submitted work for execution by a worker thread. `WorkItem` contains `data` (pointer to caller-allocated data), `data_size` (size in bytes), and `work_fn` (function pointer, `void (*)(void *)`). On success, the pool SHALL copy `item->data` via `fun_memory_copy` into internal storage and assign the copy to an idle worker slot. The pool SHALL free the internal copy after `work_fn` completes. The submit function SHALL use a round-robin starting index (`next_hint`) to scan worker slots, advancing past the slot that accepts work, to distribute consecutive submits across different workers. Pool SHALL NOT be NULL. `item` SHALL NOT be NULL. `item->data` SHALL NOT be NULL. `item->data_size` SHALL be greater than 0. `item->work_fn` SHALL NOT be NULL.

#### Scenario: Submit succeeds and data is copied
- **WHEN** `fun_thread_pool_submit` is called with a valid `WorkItem` and a worker slot is idle
- **THEN** the pool SHALL copy `item->data` of size `item->data_size` via `fun_memory_copy`, assign the copy to an idle worker's slot, signal the worker, and the result SHALL be OK

#### Scenario: Submit distributes work across workers
- **WHEN** `fun_thread_pool_submit` is called twice in succession on a pool with 2 or more workers
- **THEN** the second submit SHALL prefer a different worker slot from the first, unless the first slot has become idle and no other slot is idle

#### Scenario: Caller frees original data after submit
- **WHEN** `fun_thread_pool_submit` returns OK and the caller frees `item->data`
- **THEN** the worker SHALL process the pool's internal copy without corruption

### Requirement: Submit returns error when pool is full
When all worker threads are busy and the queue is full, `fun_thread_pool_submit` SHALL return `ERROR_CODE_THREAD_POOL_FULL` without blocking the calling thread.

#### Scenario: Submit fails when all workers are busy
- **WHEN** `fun_thread_pool_submit` is called and all workers are processing work
- **THEN** the result SHALL be `ERROR_CODE_THREAD_POOL_FULL`, no internal copy SHALL be made, and the calling thread SHALL return immediately without blocking

#### Scenario: Caller retains ownership on submit failure
- **WHEN** `fun_thread_pool_submit` returns `ERROR_CODE_THREAD_POOL_FULL`
- **THEN** the pool SHALL NOT have copied or freed `item->data` and the caller retains full ownership

### Requirement: Submit rejects NULL pool or NULL WorkItem fields
The system SHALL validate that `pool`, `item`, `item->data`, and `item->work_fn` are not NULL before copying and enqueuing work.

#### Scenario: NULL pool returns error
- **WHEN** `fun_thread_pool_submit` is called with a NULL pool
- **THEN** the function SHALL return an error result

#### Scenario: NULL item returns error
- **WHEN** `fun_thread_pool_submit` is called with a NULL `item`
- **THEN** the function SHALL return an error result

#### Scenario: NULL data returns error
- **WHEN** `fun_thread_pool_submit` is called with `item->data` NULL
- **THEN** the function SHALL return an error result

#### Scenario: NULL work_fn returns error
- **WHEN** `fun_thread_pool_submit` is called with `item->work_fn` NULL
- **THEN** the function SHALL return an error result

### Requirement: Workers execute submitted work functions with pool-owned data copies
Worker threads SHALL take ownership of `WorkItem` copies from their assigned slot and invoke the work function with the copied data pointer. Workers SHALL free the internal data copy via `fun_memory_free` after `work_fn` returns. Workers SHALL continue processing work until the pool is destroyed.

#### Scenario: Worker executes work function with copied data
- **WHEN** work is submitted with a `WorkItem`
- **THEN** a worker thread SHALL invoke `item->work_fn(copied_data)` where `copied_data` is the pool's internal copy, not the caller's original pointer

#### Scenario: Worker frees internal copy after work function completes
- **WHEN** `work_fn` returns
- **THEN** the worker SHALL call `fun_memory_free` on the internal data copy before processing the next work item

#### Scenario: Multiple workers process work concurrently
- **WHEN** work items are submitted to a pool with `num_threads > 1`
- **THEN** multiple workers SHALL process work items concurrently on different threads

### Requirement: Pool can be destroyed, waiting for workers to finish
The system SHALL provide `fun_thread_pool_destroy(pool)` that signals all workers to stop, waits for any in-progress work functions to complete, and joins all worker threads. After destroy returns, all internal resources SHALL be freed and the pool handle SHALL be invalid.

#### Scenario: Destroy waits for in-progress work to complete
- **WHEN** `fun_thread_pool_destroy` is called while a worker is executing a work function
- **THEN** the destroy call SHALL block until the work function returns

#### Scenario: Destroy on NULL pool is a no-op
- **WHEN** `fun_thread_pool_destroy` is called with a NULL pool
- **THEN** the function SHALL return without error

### Requirement: Submit is thread-safe across multiple calling threads
Multiple threads SHALL be able to call `fun_thread_pool_submit` on the same pool concurrently without data races or corruption.

#### Scenario: Concurrent submits from multiple threads succeed
- **WHEN** two or more threads call `fun_thread_pool_submit` on the same pool concurrently
- **THEN** all submitted work items SHALL be enqueued correctly without corruption and SHALL eventually be executed by workers

