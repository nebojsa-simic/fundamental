## ADDED Requirements

### Requirement: Async process spawn function
The system SHALL provide async function to spawn external processes.

#### Scenario: Spawn returns async result
- **WHEN** fun_async_process_spawn is called
- **THEN** function returns immediately with AsyncResult containing Process handle on success

### Requirement: Process handle access from AsyncResult
The system SHALL provide function to extract Process handle from AsyncResult after spawn.

#### Scenario: Get process from completed spawn result
- **WHEN** fun_async_result_get_process is called on spawn result
- **THEN** function returns pointer to Process data embedded in AsyncResult
