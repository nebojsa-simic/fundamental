# log-levels Specification

## Purpose
TBD - created by archiving change add-logging-system. Update Purpose after archive.
## Requirements
### Requirement: Log Level Hierarchy

The logging system SHALL provide five severity levels in ascending order: TRACE, DEBUG, INFO, WARN, ERROR.

#### Scenario: Level ordering
- **WHEN** log levels are compared
- **THEN** TRACE < DEBUG < INFO < WARN < ERROR

### Requirement: Compile-Time Level Filtering

The logging system SHALL compile out all log statements below the configured `FUNDAMENTAL_LOG_LEVEL`.

#### Scenario: TRACE level enabled
- **WHEN** `FUNDAMENTAL_LOG_LEVEL` is set to `LOG_LEVEL_TRACE`
- **THEN** all TRACE, DEBUG, INFO, WARN, ERROR logs are included in the binary

#### Scenario: INFO level enabled
- **WHEN** `FUNDAMENTAL_LOG_LEVEL` is set to `LOG_LEVEL_INFO`
- **THEN** TRACE and DEBUG logs compile to `((void)0)` and INFO, WARN, ERROR logs are included

#### Scenario: ERROR level only
- **WHEN** `FUNDAMENTAL_LOG_LEVEL` is set to `LOG_LEVEL_ERROR`
- **THEN** TRACE, DEBUG, INFO, WARN logs compile to `((void)0)` and only ERROR logs are included

### Requirement: Zero Overhead When Disabled

Log statements that are disabled at compile-time SHALL have zero runtime overhead.

#### Scenario: Disabled log compiles away
- **WHEN** a log statement is below `FUNDAMENTAL_LOG_LEVEL`
- **THEN** the compiled code is `((void)0)` with no function calls, no branches, no evaluation of arguments

### Requirement: Log Macro API

The logging system SHALL provide macros for each log level: `log_trace()`, `log_debug()`, `log_info()`, `log_warn()`, `log_error()`.

#### Scenario: Log macro invocation
- **WHEN** developer calls `log_info(template, params, count)`
- **THEN** the message is formatted and written if INFO level is enabled

### Requirement: No Runtime Level Changes

The logging system SHALL NOT support changing log levels at runtime.

#### Scenario: Attempting runtime change
- **WHEN** code attempts to change log level at runtime
- **THEN** no such API exists - recompilation is required

