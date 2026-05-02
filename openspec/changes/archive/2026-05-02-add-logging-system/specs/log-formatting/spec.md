## ADDED Requirements

### Requirement: Hybrid Wall-Clock Timestamp Format

Each log message SHALL include a UTC timestamp approximation in ISO 8601 format with millisecond precision, calculated from startup-time reference plus monotonic offset.

#### Scenario: Timestamp without per-call syscall
- **WHEN** a log message is written
- **THEN** the output begins with `YYYY-MM-DDTHH:MM:SS.mmmZ` (UTC time, approximated via monotonic offset)
- **AND** no system call SHALL be invoked for timestamp calculation per log call

#### Scenario: Startup captures base times
- **WHEN** logging initializes at startup
- **THEN** both wall-clock and monotonic timestamps SHALL be captured for use as reference

#### Scenario: Hybrid time calculation
- **WHEN** a log message occurs at 2026-03-27 19:45:32.123 UTC
- **THEN** the output begins with timestamp calculated as: startup_wall_time + (now_monotonic - startup_mono_time)
- **AND** accuracy SHALL be within 1000 nanoseconds of actual wall-clock

#### Scenario: Millisecond precision maintained  
- **WHEN** the calculated hybrid time has nanosecond precision
- **THEN** the output contains `T14:30:00.005Z` format (milliseconds zero-padded to 3 digits)

#### Scenario: UTC timezone maintained
- **WHEN** local wall-clock is in a different timezone
- **THEN** the output shows the equivalent UTC time from the startup reference

### Requirement: Log Level in Output

Each log message SHALL include the log level name in square brackets.

#### Scenario: Level display
- **WHEN** a log message is written at INFO level
- **THEN** the output contains `[INFO]` after the timestamp

### Requirement: Source Location in Output

Each log message SHALL include the source filename (not full path) and line number that emitted the log.

#### Scenario: Source location display
- **WHEN** `log_info("test", NULL, 0)` is called from `/home/dev/fundamental/src/network/socket.c` line 42
- **THEN** the output contains `socket.c:42` after the level (not the full path)

#### Scenario: Filename extraction from full path
- **WHEN** the source file is `/home/dev/fundamental/src/network/socket.c`
- **THEN** only `socket.c` is displayed (path stripped, just filename)

### Requirement: Message Formatting via fun_string_template

Log messages SHALL be formatted using `fun_string_template()` with the provided parameters.

#### Scenario: Template with parameters
- **WHEN** `log_info("User #{id} logged in", params, 1)` is called with `params[0] = {"id", {.intValue = 42}}`
- **THEN** the output contains `User 42 logged in`

#### Scenario: Template with no parameters
- **WHEN** `log_info("Simple message", NULL, 0)` is called
- **THEN** the output contains `Simple message`

#### Scenario: Template with missing parameter
- **WHEN** template references a parameter not in the params array
- **THEN** the placeholder is replaced with empty string (fun_string_template behavior)

### Requirement: Complete Output Format

The complete log line format SHALL be:
```
[ISO8601_TIMESTAMP] [LEVEL] filename:line formatted_message
```

#### Scenario: Complete log line
- **WHEN** `log_info("User #{id} from ${ip}", params, 2)` is called at 2026-03-27T19:45:32.123Z from `/home/dev/fundamental/src/app.c:10` with `id=42, ip="192.168.1.1"`
- **THEN** the output is `2026-03-27T19:45:32.123Z [INFO] app.c:10 User 42 from 192.168.1.1`

### Requirement: Newline Termination

Each log message SHALL be terminated with a newline character.

#### Scenario: Newline at end
- **WHEN** a log message is written
- **THEN** the output ends with `\n`

### Requirement: Buffer Size Configuration

The logging system SHALL use a compile-time configurable buffer size via `FUNDAMENTAL_LOG_BUFFER_SIZE` for formatted messages.

#### Scenario: Default buffer size
- **WHEN** `FUNDAMENTAL_LOG_BUFFER_SIZE` is not defined
- **THEN** a default of 512 bytes is used

#### Scenario: Custom buffer size
- **WHEN** `FUNDAMENTAL_LOG_BUFFER_SIZE` is set to `1024`
- **THEN** formatted messages up to 1023 characters are supported

#### Scenario: Message exceeds buffer
- **WHEN** a formatted message exceeds the buffer size
- **THEN** the message is truncated (behavior of fun_string_template with small buffer)
