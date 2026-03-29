## ADDED Requirements

### Requirement: Centralized Shutdown Dispatcher

The system SHALL provide a centralized shutdown function `fun_shutdown_run()` that executes all module deinitializers in reverse order from initialization.

#### Scenario: Shutdown runs registered cleanup functions
- **WHEN** `fun_shutdown_run()` is called
- **THEN** all previously registered cleanup functions SHALL execute in reverse initialization order

#### Scenario: Single execution guarantee
- **WHEN** the shutdown sequence begins
- **THEN** each registered cleanup function SHALL execute exactly once

#### Scenario: Idempotent shutdown calls
- **WHEN** `fun_shutdown_run()` is called multiple times
- **THEN** it SHALL be safe to call (no side effects after first call)
- **AND** subsequent calls SHALL return without executing cleanup again

### Requirement: Reverse Phase Execution

Module cleanup functions SHALL execute in the reverse order of their corresponding initialization phases.

#### Scenario: Network module shuts down last
- **WHEN** shutdown occurs
- **THEN** modules that initiated at startup last (NETWORK: phase 6) SHALL clean up first (SHUTDOWN_PHASE_NETWORK)
- **AND** modules that initiated first at startup (PLATFORM: phase 1) SHALL clean up last (SHUTDOWN_PHASE_PLATFORM)

#### Scenario: Application modules retain control
- **WHEN** application layers shut down
- **THEN** they SHALL execute at SHUTDOWN_PHASE_APP (phase 99)
- **AND** infrastructure remains available during their cleanup

### Requirement: Shutdown Function Registration

Modules SHALL register cleanup functions for execution during shutdown using a registration macro.

#### Scenario: Module registers cleanup function
- **WHEN** a module needs deinitialization
- **THEN** it SHALL use `FUNDAMENTAL_SHUTDOWN_REGISTER(phase, function)` macro

#### Scenario: Application registers custom cleanup
- **WHEN** an application needs to run custom cleanup
- **THEN** it SHALL register at `SHUTDOWN_PHASE_APP`
- **AND** its cleanup function SHALL run before infrastructure cleanup

### Requirement: Exit Code Support

The shutdown function SHALL accept an exit code parameter for proper process termination.

#### Scenario: Shutdown with success exit code
- **WHEN** shutdown is called as part of normal exit sequence
- **THEN** `fun_shutdown_run()` MAY be called with exit code `0`

#### Scenario: Shutdown with error exit code
- **WHEN** shutdown is called after application encounters an error
- **THEN** `fun_shutdown_run()` MAY be called with appropriate error exit code

#### Scenario: Platform-specific exit code mapping
- **WHEN** shutdown is triggered by platform-specific signal
- **THEN** appropriate standardized exit codes SHALL be used (e.g., 128+signal_num for Unix signals)

### Requirement: Atomic Operation Protection

The shutdown system SHALL protect against race conditions during execution.

#### Scenario: Multiple shutdown initiation calls
- **WHEN** multiple threads attempt to initiate shutdown simultaneously
- **THEN** only one shall proceed with cleanup execution
- **AND** others shall return immediately without attempting cleanup

#### Scenario: Atomic shutdown flag operations
- **WHEN** `fun_shutdown_run()` executes
- **THEN** shutdown state flags SHALL be operated on atomically
- **AND** `__atomic_compare_exchange_n` or equivalent SHALL be used