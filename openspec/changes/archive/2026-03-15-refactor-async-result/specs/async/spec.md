## MODIFIED Requirements

### Requirement: Single operation await
Async module SHALL provide `fun_async_await` to suspend execution until a single operation completes or a timeout elapses.

#### Scenario: Wait for pending operation that completes
- **WHEN** `fun_async_await` is called with a pending `AsyncResult` and `timeout_ms = -1`
- **THEN** the function SHALL block until the operation completes and `result->status` SHALL be `ASYNC_COMPLETED`

#### Scenario: Wait times out
- **WHEN** `fun_async_await` is called with `timeout_ms > 0` and the operation does not complete within that duration
- **THEN** `fun_async_await` SHALL return an error result with `ERROR_CODE_ASYNC_TIMEOUT` and `result->status` SHALL be set to `ASYNC_ERROR`

#### Scenario: Poll once with zero timeout
- **WHEN** `fun_async_await` is called with `timeout_ms = 0`
- **THEN** the function SHALL invoke `poll` exactly once and return immediately regardless of whether the operation completed

#### Scenario: Operation errors before timeout
- **WHEN** `fun_async_await` is called and the operation's `poll` function sets `status` to `ASYNC_ERROR`
- **THEN** `fun_async_await` SHALL return that error immediately without waiting for the timeout

### Requirement: Multiple operation await
Async module SHALL provide `fun_async_await_all` to suspend execution until all operations complete or the timeout elapses.

#### Scenario: All operations complete within timeout
- **WHEN** `fun_async_await_all` is called with an array of pending `AsyncResult` objects and `timeout_ms = -1`
- **THEN** the function SHALL block until every operation has `status` of `ASYNC_COMPLETED` or `ASYNC_ERROR`

#### Scenario: Timeout expires before all complete
- **WHEN** `fun_async_await_all` is called with `timeout_ms > 0` and one or more operations do not complete in time
- **THEN** each incomplete operation SHALL have `status` set to `ASYNC_ERROR` and `error` set to `ERROR_CODE_ASYNC_TIMEOUT`
- **AND** `fun_async_await_all` SHALL return an error result with `ERROR_CODE_ASYNC_TIMEOUT`

## REMOVED Requirements

### Requirement: Async process spawn function
**Reason**: Process spawning moves to the dedicated `process` module. `async` is a pure future primitive with no knowledge of specific operations.
**Migration**: Replace `fun_async_process_spawn(exe, args, opts)` with `fun_process_spawn(exe, args, opts, &proc_result)` from `include/process/process.h`.

### Requirement: Process handle access from AsyncResult
**Reason**: `AsyncResult` no longer embeds `Process`. Process output is accessed directly via `ProcessResult` fields in the `process` module.
**Migration**: Replace `fun_async_result_get_process(&result)` with direct access to the `ProcessResult *` passed to `fun_process_spawn`.
