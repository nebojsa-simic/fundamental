## ADDED Requirements

### Requirement: Event loop can be created and destroyed
The system SHALL provide a `fun_network_loop_init` function that initialises a caller-allocated `NetworkLoop` and a `fun_network_loop_destroy` function that releases all platform resources associated with it.

#### Scenario: Successful initialisation
- **WHEN** the caller provides a valid `NetworkLoop` buffer of at least `NETWORK_LOOP_SIZE` bytes
- **THEN** `fun_network_loop_init` SHALL return a result with no error and the loop SHALL be ready to accept connections

#### Scenario: Destroy releases resources
- **WHEN** `fun_network_loop_destroy` is called on an initialised loop
- **THEN** all platform handles (epoll fd / IOCP handle) SHALL be closed and no resources SHALL be leaked

### Requirement: Event loop can run in blocking mode
The system SHALL provide a `fun_network_loop_run` function that blocks the calling thread, dispatching I/O callbacks until `fun_network_loop_stop` is called.

#### Scenario: Run processes pending events
- **WHEN** `fun_network_loop_run` is called and I/O events are available
- **THEN** the corresponding connection callbacks SHALL be invoked before the function returns

#### Scenario: Run blocks until stopped
- **WHEN** `fun_network_loop_run` is called with no pending events
- **THEN** the function SHALL block until either an event arrives or `fun_network_loop_stop` is called from a callback

### Requirement: Event loop can tick with optional timeout
The system SHALL provide a `fun_network_loop_run_once` function that accepts a `timeout_ms` parameter. It SHALL wait up to `timeout_ms` milliseconds for I/O events, dispatch any ready callbacks, and return. A timeout of 0 means return immediately (pure poll). A timeout of -1 means block indefinitely until at least one event arrives.

#### Scenario: Run-once with zero timeout returns immediately
- **WHEN** `fun_network_loop_run_once` is called with `timeout_ms = 0` and no events are ready
- **THEN** the function SHALL return immediately without blocking

#### Scenario: Run-once with positive timeout waits then returns
- **WHEN** `fun_network_loop_run_once` is called with `timeout_ms = 100` and no events arrive within 100ms
- **THEN** the function SHALL return after approximately 100ms

#### Scenario: Run-once dispatches ready callbacks
- **WHEN** `fun_network_loop_run_once` is called and one or more connections have ready I/O
- **THEN** their callbacks SHALL be invoked before the function returns

#### Scenario: Run-once with negative-one timeout blocks until event
- **WHEN** `fun_network_loop_run_once` is called with `timeout_ms = -1` and no events are ready
- **THEN** the function SHALL block until at least one event arrives or `fun_network_loop_stop` is called

### Requirement: Event loop can be stopped
The system SHALL provide a `fun_network_loop_stop` function that signals the loop to exit its `fun_network_loop_run` call.

#### Scenario: Stop causes run to return
- **WHEN** `fun_network_loop_stop` is called from within an I/O callback while the loop is running
- **THEN** `fun_network_loop_run` SHALL return after completing the current dispatch cycle
