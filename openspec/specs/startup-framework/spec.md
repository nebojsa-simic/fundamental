# startup-framework Specification

## Purpose
TBD - created by archiving change centralized-startup-init. Update Purpose after archive.
## Requirements
### Requirement: Centralized Startup Dispatcher

The system SHALL provide a centralized startup initialization function `fun_startup_run()` that executes all module initializers in a defined order, called from `__main()`.

#### Scenario: Startup runs via __main()
- **WHEN** the program starts
- **THEN** `__main()` SHALL be called by GCC-generated code before `main()`

#### Scenario: __main() calls dispatcher
- **WHEN** `__main()` is called
- **THEN** it SHALL call `fun_startup_run()` to execute all initialization

#### Scenario: Controlled initialization order
- **WHEN** multiple modules require initialization
- **THEN** they SHALL execute in the defined phase order (platform → memory → filesystem → config → logging → network)

#### Scenario: Single execution guarantee
- **WHEN** the program starts
- **THEN** each initialization function SHALL execute exactly once

### Requirement: Initialization Phases

The system SHALL define numbered initialization phases that determine execution order.

#### Scenario: Phase ordering
- **WHEN** phases are defined
- **THEN** lower-numbered phases SHALL execute before higher-numbered phases

#### Scenario: Phase 1 - Platform
- **WHEN** startup begins
- **THEN** phase 1 (platform) SHALL initialize first

#### Scenario: Phase 4 - Config
- **WHEN** startup reaches phase 4
- **THEN** config SHALL load `fun.ini` before logging and network initialize

#### Scenario: Phase 5 - Logging
- **WHEN** startup reaches phase 5
- **THEN** logging SHALL initialize after config (can read `[logging]` section)

#### Scenario: Phase 6 - Network
- **WHEN** startup reaches phase 6
- **THEN** network SHALL initialize after config (can read `[network]` section)

### Requirement: Module Registration

Modules SHALL register their initialization functions with the startup dispatcher.

#### Scenario: Registration via macro
- **WHEN** a module needs initialization
- **THEN** it SHALL use `FUNDAMENTAL_STARTUP_REGISTER(phase, function)` macro

#### Scenario: Static registration
- **WHEN** the program is compiled
- **THEN** all init functions SHALL be in a static array (no dynamic registration)

### Requirement: Fail Fast on Error

If any initialization phase fails, the system SHALL abort immediately.

#### Scenario: Platform init failure
- **WHEN** `fun_platform_init()` returns error
- **THEN** the system SHALL print diagnostic and abort (not continue to other phases)

#### Scenario: Config init failure
- **WHEN** config loading fails
- **THEN** the system SHALL use defaults and continue (config failure is non-fatal)

#### Scenario: Logging init failure
- **WHEN** logging initialization fails
- **THEN** the system SHALL continue without logging (logging failure is non-fatal)

### Requirement: Optional Startup Diagnostics

The system SHALL provide compile-time verbose output for debugging startup sequence.

#### Scenario: Verbose mode enabled
- **WHEN** compiled with `-DFUNDAMENTAL_STARTUP_VERBOSE=1`
- **THEN** each phase SHALL print its name before executing

#### Scenario: Verbose mode disabled
- **WHEN** compiled without `FUNDAMENTAL_STARTUP_VERBOSE`
- **THEN** startup SHALL be silent (no diagnostic output)

### Requirement: No Circular Dependencies

The system SHALL prevent circular dependencies between modules during initialization.

#### Scenario: Config does not use logging
- **WHEN** config module initializes (phase 4)
- **THEN** it SHALL NOT call any logging functions (logging not yet initialized)

#### Scenario: Logging uses config
- **WHEN** logging module initializes (phase 5)
- **THEN** it MAY read config values (config already initialized)

