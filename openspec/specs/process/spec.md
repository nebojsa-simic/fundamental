# Process Module Specification
## Purpose
Process module provides functions to spawn, monitor, terminate, and free external processes. Process spawning is decoupled from the async module; `AsyncResult` is a pure future primitive with no knowledge of processes.

## Requirements

### Requirement: Process can be spawned asynchronously
The system SHALL provide `fun_process_spawn` in the `process` module that launches an external process and returns an `AsyncResult`. The caller SHALL provide a `ProcessResult *` to receive all process output.

#### Scenario: Spawn returns immediately with async handle
- **WHEN** `fun_process_spawn` is called with a valid executable, arguments, and a non-NULL `ProcessResult *`
- **THEN** the function SHALL return immediately with an `AsyncResult` in `ASYNC_PENDING` state
- **AND** `out->_handle` SHALL be set to the OS process handle immediately (before await)

#### Scenario: Process output written on completion
- **WHEN** `fun_async_await` completes on a spawn result without error
- **THEN** `out->stdout_length` SHALL contain the number of bytes written to `out->stdout_data`
- **AND** `out->stderr_length` SHALL contain the number of bytes written to `out->stderr_data`
- **AND** `out->exit_code` SHALL contain the process exit code

#### Scenario: Output truncated when buffer capacity exceeded
- **WHEN** a process produces more stdout bytes than `out->stdout_capacity`
- **THEN** the library SHALL write at most `out->stdout_capacity` bytes and set `out->stdout_length` to `out->stdout_capacity`

### Requirement: ProcessResult is a caller-allocated output type
The system SHALL define `ProcessResult` with pointer+capacity+length fields for stdout and stderr. The caller sets buffer pointers and capacities before spawning; the library fills lengths and exit code on completion.

#### Scenario: Caller initialises stdout buffer before spawn
- **WHEN** a caller sets `out->stdout_data` to a buffer pointer and `out->stdout_capacity` to its size before calling `fun_process_spawn`
- **THEN** the library SHALL write captured stdout into that buffer on completion

#### Scenario: Caller initialises stderr buffer before spawn
- **WHEN** a caller sets `out->stderr_data` to a buffer pointer and `out->stderr_capacity` to its size before calling `fun_process_spawn`
- **THEN** the library SHALL write captured stderr into that buffer on completion

### Requirement: Running process can be terminated
The system SHALL provide `fun_process_terminate` that accepts a `ProcessResult *` and forcefully terminates the associated process.

#### Scenario: Terminate kills running process
- **WHEN** `fun_process_terminate` is called on a `ProcessResult` whose process is still running
- **THEN** the process SHALL be terminated and subsequent `fun_async_await` SHALL complete

#### Scenario: Terminate on already-completed process returns no error
- **WHEN** `fun_process_terminate` is called after the process has already exited
- **THEN** the function SHALL return a result with no error

### Requirement: Process resources can be freed
The system SHALL provide `fun_process_free` that accepts a `ProcessResult *` and releases all OS handles associated with the process.

#### Scenario: Free releases OS handles after completion
- **WHEN** `fun_process_free` is called after `fun_async_await` completes
- **THEN** all OS handles held in `_handle` SHALL be released and no resources SHALL be leaked

### Requirement: Process accessors are replaced by direct field access
The process module SHALL NOT provide `fun_process_get_stdout`, `fun_process_get_stderr`, `fun_process_get_exit_code`, or `fun_async_result_get_process`. Callers access process output directly via `ProcessResult` fields.

#### Scenario: stdout accessed via field after completion
- **WHEN** a caller needs captured stdout after spawn completes
- **THEN** the caller SHALL read `out->stdout_data[0 .. out->stdout_length]` directly

#### Scenario: exit code accessed via field after completion
- **WHEN** a caller needs the exit code after spawn completes
- **THEN** the caller SHALL read `out->exit_code` directly
