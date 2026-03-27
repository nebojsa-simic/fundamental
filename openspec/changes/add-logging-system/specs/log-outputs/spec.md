## ADDED Requirements

### Requirement: Console Output

The logging system SHALL support writing log messages to stdout via `fun_console_write_line()`.

#### Scenario: Console output enabled
- **WHEN** `FUNDAMENTAL_LOG_OUTPUT_CONSOLE` is set to `1`
- **THEN** all enabled log messages are written to stdout

#### Scenario: Console output disabled
- **WHEN** `FUNDAMENTAL_LOG_OUTPUT_CONSOLE` is set to `0`
- **THEN** no output is written to stdout

### Requirement: File Output

The logging system SHALL support writing log messages to a file in append mode using Fundamental's file operations, not stdio functions.

#### Scenario: File output enabled using Fundamental API
- **WHEN** `FUNDAMENTAL_LOG_OUTPUT_FILE` is set to `1` and `FUNDAMENTAL_LOG_FILE_PATH` is set
- **THEN** all enabled log messages are appended to the specified file
- **AND** `fun_file_*` functions SHALL be used (not `fopen`, `fprintf`, `fclose`)

#### Scenario: File handle opened once at startup
- **WHEN** logging initializes (at startup)
- **THEN** `fun_file_open_append()` SHALL be called once to obtain the file handle
- **AND** the handle SHALL be stored in a static variable for reuse

#### Scenario: File output disabled
- **WHEN** `FUNDAMENTAL_LOG_OUTPUT_FILE` is set to `0`
- **THEN** no file I/O occurs

#### Scenario: File write uses persistent handle
- **WHEN** a log message is to be written to file
- **THEN** `fun_file_write()` SHALL be called with the cached file handle
- **AND** the file handle SHALL NOT be closed after each write (for performance)

#### Scenario: File open failure
- **WHEN** file output is enabled but the file cannot be opened via `fun_file_open_append()`
- **THEN** the error is silently ignored (logging continues on console only)

#### Scenario: Shutdown cleanup
- **WHEN** the application shuts down (if shutdown handlers are available)
- **THEN** the file handle MAY be closed via `fun_file_close()`

### Requirement: Dual Output

The logging system SHALL support writing to both console and file simultaneously.

#### Scenario: Both outputs enabled
- **WHEN** both `FUNDAMENTAL_LOG_OUTPUT_CONSOLE` and `FUNDAMENTAL_LOG_OUTPUT_FILE` are `1`
- **THEN** each log message is written to both stdout and the file

### Requirement: Configurable File Path

The logging system SHALL accept a compile-time configurable file path via `FUNDAMENTAL_LOG_FILE_PATH`.

#### Scenario: Custom log file path
- **WHEN** `FUNDAMENTAL_LOG_FILE_PATH` is set to `"/var/log/myapp.log"`
- **THEN** logs are written to that path

### Requirement: Synchronous Writes

All log output SHALL be written synchronously (blocking) before the log macro returns.

#### Scenario: Log write ordering
- **WHEN** multiple log statements are executed in sequence
- **THEN** logs appear in the output in the same order as the statements

### Requirement: No Network Output

The logging system SHALL NOT support network output targets in this version.

#### Scenario: Network logging attempt
- **WHEN** code attempts to configure network logging
- **THEN** no such configuration option exists
