# log-config Specification

## Purpose
TBD - created by archiving change add-logging-system. Update Purpose after archive.
## Requirements
### Requirement: Automatic Initialization at Startup

The logging system SHALL auto-initialize at program startup via the `__main()` dispatcher — no explicit `log_init()` required by the user and no per-call initialization check.

#### Scenario: Startup runs via __main()
- **WHEN** the program starts
- **THEN** `__main()` SHALL be called by GCC-generated code before `main()`

#### Scenario: __main() calls startup dispatcher
- **WHEN** `__main()` is called
- **THEN** it SHALL call `fun_startup_run()` which initializes logging at phase 5

#### Scenario: No user-facing init function required
- **WHEN** a developer uses the logging system
- **THEN** they SHALL NOT need to call any `log_init()` or similar function

#### Scenario: No per-call initialization overhead
- **WHEN** a log macro is invoked
- **THEN** there SHALL be no branch or check for initialization (already complete)

#### Scenario: Config loaded once at startup
- **WHEN** application starts
- **THEN** configuration SHALL be loaded and cached for the process lifetime

#### Scenario: Runtime config change does NOT take effect
- **WHEN** `fun.ini` is modified while the application is running
- **THEN** the logging system SHALL continue using the cached values from startup

#### Scenario: Config change takes effect after restart
- **WHEN** `fun.ini` is modified AND the application is restarted
- **THEN** the new configuration values SHALL be loaded and used

#### Scenario: Config loading failure
- **WHEN** `fun.ini` cannot be loaded or parsed
- **THEN** compile-time defaults SHALL be used for all settings

### Requirement: No Logging in Config Module

The logging system SHALL NOT be used by the config module during configuration loading.

#### Scenario: Config loads without logging
- **WHEN** `fun_config_load()` is called
- **THEN** it SHALL NOT call any logging functions

#### Scenario: Circular dependency avoided
- **WHEN** logging initializes and loads config
- **THEN** config loading SHALL NOT attempt to log (would cause infinite recursion)

