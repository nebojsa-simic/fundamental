## 1. Project Setup

- [ ] 1.1 Create `include/fundamental/logging/logging.h` header file
- [ ] 1.2 Create `src/logging/logging.c` implementation file
- [ ] 1.3 Create `arch/logging/linux-amd64/logging.c` for Linux timestamp
- [ ] 1.4 Create `arch/logging/windows-amd64/logging.c` for Windows timestamp
- [ ] 1.5 Add compile-time configuration defines in header

## 2. Log Level Infrastructure

- [ ] 2.1 Define `LOG_LEVEL_TRACE`, `LOG_LEVEL_DEBUG`, `LOG_LEVEL_INFO`, `LOG_LEVEL_WARN`, `LOG_LEVEL_ERROR` constants
- [ ] 2.2 Implement `FUNDAMENTAL_LOG_LEVEL` compile-time configuration
- [ ] 2.3 Create level-to-string conversion function
- [ ] 2.4 Implement conditional compilation macros for each level
- [ ] 2.5 Add runtime level filtering from fun.ini config

## 3. Hybrid Timestamp Implementation

- [ ] 3.1 Implement single startup call to fetch accurate UTC timestamp
- [ ] 3.2 Implement single startup call to fetch monotonic timestamp
- [ ] 3.3 Store both timestamps in static variables at startup
- [ ] 3.4 Implement runtime function: base_wall_time + (now_monotonic - base_mono)
- [ ] 3.5 Verify no syscall per log call (time calculation is arithmetic only)
- [ ] 3.6 Create formatter for hybrid time as ISO 8601: `2026-03-27T19:45:32.123Z`
- [ ] 3.7 Verify timing accuracy vs actual wall-clock (within 1000ns)

## 4. Log Formatting

- [ ] 4.1 Implement internal `log_format_message()` using `fun_string_template()`
- [ ] 4.2 Implement `LOG_BASENAME` macro to strip path from `__FILE__`
- [ ] 4.3 Add source location capture (`LOG_BASENAME(__FILE__)`, `__LINE__`) to log macros
- [ ] 4.4 Format complete log line: `[timestamp] [LEVEL] filename:line message`
- [ ] 4.5 Handle buffer overflow gracefully (truncate)

## 5. fun.ini Configuration

- [ ] 5.1 Add `#include "../config/config.h"` for config loading
- [ ] 5.2 Implement `logging_init()` function registered with centralized startup
- [ ] 5.3 Register logging init at `STARTUP_PHASE_LOGGING` via `FUNDAMENTAL_STARTUP_REGISTER()`
- [ ] 5.4 Load `[logging]` section from fun.ini via `fun_config_load()`
- [ ] 5.5 Cache config values in static variables (one-time load at startup)
- [ ] 5.6 Support `level` key (TRACE, DEBUG, INFO, WARN, ERROR)
- [ ] 5.7 Support `output_console` key (boolean)
- [ ] 5.8 Support `output_file` key (boolean)
- [ ] 5.9 Support `file_path` key (string)
- [ ] 5.10 Support `buffer_size` key (integer, minimum 128)
- [ ] 5.11 Fall back to compile-time defaults when config missing
- [ ] 5.12 Document: Config module MUST NOT call logging functions (circular dependency)

## 6. Output Implementation

- [ ] 6.1 Implement console output using `fun_console_write_line()`
- [ ] 6.2 Implement file output using `fun_file_open_append()` to get persistent handle
- [ ] 6.3 Store file handle in static variable during logging init
- [ ] 6.4 Write to cached handle via `fun_file_write()` during each log call
- [ ] 6.5 Add runtime check for `output_console` config value
- [ ] 6.6 Add runtime check for `output_file` config value
- [ ] 6.7 Implement dual output (console + file simultaneously)
- [ ] 6.8 Close file handle at shutdown (optional cleanup)

## 7. Log Macro API

- [ ] 7.1 Implement `log_trace(template, params, count)` macro
- [ ] 7.2 Implement `log_debug(template, params, count)` macro
- [ ] 7.3 Implement `log_info(template, params, count)` macro
- [ ] 7.4 Implement `log_warn(template, params, count)` macro
- [ ] 7.5 Implement `log_error(template, params, count)` macro
- [ ] 7.6 Ensure disabled macros compile to `((void)0)`

## 8. Testing

- [ ] 8.1 Create `tests/logging/` test directory
- [ ] 8.2 Create `tests/logging/build-linux-amd64.sh` build script
- [ ] 8.3 Create `tests/logging/build-windows-amd64.bat` build script
- [ ] 8.4 Write test: log levels compile out correctly
- [ ] 8.5 Write test: console output works
- [ ] 8.6 Write test: file output works
- [ ] 8.7 Write test: timestamp format is correct
- [ ] 8.8 Write test: template formatting works
- [ ] 8.9 Write test: source location is captured
- [ ] 8.10 Write test: LOG_BASENAME strips path (shows only filename)
- [ ] 8.11 Write test: fun.ini config overrides defaults
- [ ] 8.12 Write test: missing config uses compile-time defaults
- [ ] 8.13 Write test: config loaded once (not re-read on file change)
- [ ] 8.14 Write test: auto-init on first log call (no init function needed)
- [ ] 8.15 Run tests on Linux and verify pass

## 9. Documentation

- [ ] 9.1 Add usage examples to header file
- [ ] 9.2 Document compile-time configuration options
- [ ] 9.3 Document fun.ini `[logging]` section options
- [ ] 9.4 Add example fun.ini snippet to README
