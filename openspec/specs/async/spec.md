# Async Module Specification 
## Purpose  
Asynchronous operations module provides polling mechanism for non-blocking execution allowing applications to  
perform other work while waiting for operations.  

## Requirements

### Requirement: Single operation await
Async module SHALL provide function to suspend execution until single operation completes.

#### Scenario: Wait for pending operation
- **WHEN** fun_async_await is called with pending AsyncResult
- **THEN** function blocks until operation completes or errors

### Requirement: Multiple operation await
Async module SHALL provide function to suspend execution until all operations in a collection complete.

#### Scenario: Wait for multiple pending operations
- **WHEN** fun_async_await_all is called with array of pending AsyncResult objects
- **THEN** function blocks until all operations complete or error
- **AND** each individual AsyncResult is polled independently during waiting
- **AND** function returns when no operations remain pending

### Requirement: Async process spawn function
Async module SHALL provide async function to spawn external processes.

#### Scenario: Spawn returns async result
- **WHEN** fun_async_process_spawn is called
- **THEN** function returns immediately with AsyncResult containing Process handle on success

### Requirement: Process handle access from AsyncResult
Async module SHALL provide function to extract Process handle from AsyncResult after spawn.

#### Scenario: Get process from completed spawn result
- **WHEN** fun_async_result_get_process is called on spawn result
- **THEN** function returns pointer to Process data embedded in AsyncResult

## Constraints
- Functions MUST avoid busy-waiting when possible
- AsyncResult.status SHALL be updated during polling operations
- Poll function callbacks MUST be invoked to update operation status
