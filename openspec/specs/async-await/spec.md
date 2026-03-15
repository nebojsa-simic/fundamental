# Async Await Specification
## Purpose
Defines the contract for the `fun_async_await` and `fun_async_await_all` timeout parameter and return type.

## Requirements

### Requirement: Await has a timeout parameter
`fun_async_await` and `fun_async_await_all` SHALL accept a `timeout_ms` parameter controlling the maximum wait duration.

#### Scenario: Negative one means block indefinitely
- **WHEN** `fun_async_await` is called with `timeout_ms = -1`
- **THEN** the function SHALL not return until the operation reaches `ASYNC_COMPLETED` or `ASYNC_ERROR`

#### Scenario: Zero means poll once and return
- **WHEN** `fun_async_await` is called with `timeout_ms = 0`
- **THEN** the function SHALL call `poll` exactly once and return immediately

#### Scenario: Positive value is a deadline in milliseconds
- **WHEN** `fun_async_await` is called with `timeout_ms = N` where N > 0
- **THEN** the function SHALL return no later than N milliseconds after it was called

### Requirement: Await returns CanReturnError(void)
`fun_async_await` and `fun_async_await_all` SHALL return `CanReturnError(void)` so timeout and other terminal errors propagate to the caller.

#### Scenario: Successful completion returns no error
- **WHEN** `fun_async_await` returns and the operation status is `ASYNC_COMPLETED`
- **THEN** the returned `voidResult` SHALL have `error.code == ERROR_CODE_NO_ERROR`

#### Scenario: Timeout returns error
- **WHEN** `fun_async_await` times out
- **THEN** the returned `voidResult` SHALL have `error.code == ERROR_CODE_ASYNC_TIMEOUT`

#### Scenario: Operation error propagates
- **WHEN** the operation's `poll` sets `status` to `ASYNC_ERROR`
- **THEN** `fun_async_await` SHALL return that same error in the `voidResult`

### Requirement: ERROR_CODE_ASYNC_TIMEOUT is defined
The error module SHALL define `ERROR_CODE_ASYNC_TIMEOUT` for use when an await deadline expires.

#### Scenario: Timeout error code is distinct
- **WHEN** a timeout occurs during `fun_async_await`
- **THEN** `result.error.code` SHALL equal `ERROR_CODE_ASYNC_TIMEOUT` which SHALL be distinct from all other error codes
