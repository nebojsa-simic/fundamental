## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    LOGGING ARCHITECTURE                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  COMPILE-TIME CONFIGURATION                                     │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  #define FUNDAMENTAL_LOG_LEVEL LOG_LEVEL_INFO           │    │
│  │  #define FUNDAMENTAL_LOG_OUTPUT_CONSOLE 1               │    │
│  │  #define FUNDAMENTAL_LOG_OUTPUT_FILE 0                  │    │
│  └─────────────────────────────────────────────────────────┘    │
│                              │                                  │
│                              ▼                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  MACROS (compile out when disabled)                     │    │
│  │                                                         │    │
│  │  log_trace(...) ──► ((void)0) when level > TRACE        │    │
│  │  log_debug(...) ──► ((void)0) when level > DEBUG        │    │
│  │  log_info(...)  ──► ((void)0) when level > INFO         │    │
│  │  log_warn(...)  ──► ((void)0) when level > WARN         │    │
│  │  log_error(...) ──► ((void)0) when level > ERROR        │    │
│  └─────────────────────────────────────────────────────────┘    │
│                              │                                  │
│                              ▼                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  SYNCHRONOUS WRITER                                     │    │
│  │                                                         │    │
│  │  1. Build timestamp (monotonic)                         │    │
│  │  2. Call fun_string_template()                          │    │
│  │  3. Write to console and/or file                        │    │
│  └─────────────────────────────────────────────────────────┘    │
│                              │                                  │
│              ┌───────────────┴───────────────┐                  │
│              ▼                               ▼                  │
│  ┌──────────────────┐            ┌──────────────────┐           │
│  │ fun_console_     │            │ File writer      │           │
│  │ write_line()     │            │ (append-style)   │           │
│  └──────────────────┘            └──────────────────┘           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Design Decisions

### 0. Configuration: Compile-Time Defaults with fun.ini Overrides

**Decision**: All logging configuration has compile-time defaults, but can be overridden at runtime via `fun.ini`.

```ini
; fun.ini example
[logging]
level = INFO
output_console = 1
output_file = 1
file_path = /var/log/myapp.log
buffer_size = 1024
```

**Rationale**:
- Compile-time defaults ensure zero overhead when logging is completely disabled
- Runtime config allows tuning without recompilation (buffer sizes, output targets)
- Follows existing pattern from network module (`rx_buf_size` config)
- Config loaded once at application startup, cached for log macro use

**Tradeoff**: Log level requires recompilation to change (as designed), but buffer sizes and file paths can be tuned via config.

**Constraint**: The config module MUST NOT call logging functions during initialization. This creates a circular dependency. Document this as: "Do not use logging in fun_config_load() or related functions."

---

## Design Decisions

### 1. Synchronous Only (No Threading)

**Decision**: Logging is synchronous and blocking. No background threads, no async queues.

**Rationale**:
- Threading primitives not yet in Fundamental Library
- KISS principle — start with simplest working solution
- Synchronous writes ensure logs are never lost on crash
- Can add async buffering later when threading exists

**Tradeoff**: Logging latency adds to operation latency. Acceptable for now.

---

### 2. Compile-Time Configuration with Runtime Overrides

**Decision**: Logging has compile-time defaults that can be overridden by `fun.ini` at runtime.

**Compile-time defaults** (in header or build flags):
```c
#define FUNDAMENTAL_LOG_LEVEL LOG_LEVEL_INFO
#define FUNDAMENTAL_LOG_OUTPUT_CONSOLE 1
#define FUNDAMENTAL_LOG_OUTPUT_FILE 0
#define FUNDAMENTAL_LOG_FILE_PATH "/tmp/app.log"
#define FUNDAMENTAL_LOG_BUFFER_SIZE 512
```

**Runtime overrides** (via `fun.ini`):
```ini
[logging]
level = DEBUG
output_console = 1
output_file = 1
file_path = /var/log/myapp.log
buffer_size = 1024
```

**How it works**:
1. Compile-time defines set the defaults
2. At application startup (`__main()` → `fun_startup_run()`), config is loaded from `fun.ini`
3. Values are cached in static variables for the lifetime of the process
4. Config is NOT re-read — changes require application restart

**Exception**: `FUNDAMENTAL_LOG_LEVEL` controls what compiles in. If set to `INFO`, DEBUG logs are `((void)0)` at compile time — no runtime config can enable them. However, you can compile with `FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_TRACE` and then use `fun.ini` to set `level = WARN` to suppress verbose output without recompiling.

**Rationale**:
- Compile-time filtering for log level ensures zero overhead for disabled levels
- Runtime config (via `fun.ini`) allows tuning buffer sizes, file paths without recompilation
- Single load at startup — no repeated file I/O, no dynamic reloading complexity
- Changes to `fun.ini` take effect on next application start (standard for INI-based config)
- Follows existing network module pattern (`rx_buf_size` loaded once at reactor init)

**Constraint**: Config module MUST NOT use logging. Circular dependency would break both.

---

### 3. Startup Initialization via __main()

**Decision**: Use the `__main()` stub and centralized startup dispatcher to initialize logging at program startup — no per-call branch check.

**Implementation pattern**:
```c
// src/startup/startup.c
void __main(void)
{
    fun_startup_run();  // Calls all phase inits including logging
}

// In fun_startup_run() phase 5:
fun_logging_init();  // Loads config, caches values

// logging.h
#define log_info(template, params, count) \
    do { \
        /* No init check - already initialized at startup */ \
        /* actual logging logic */ \
    } while (0)
```

**Rationale**:
- Zero overhead per log call — no branch, no check
- `__main()` runs before `main()`, guaranteed single execution
- Uses existing GCC-generated `__main()` call mechanism
- No GCC `__attribute__((constructor))` complexity
- Centralized with other module initialization

**Tradeoff**: `__main()` runs even if application never logs. Negligible cost (~1ms at startup).

**Note**: The `logging_initialized` flag is retained for debugging/safety but not checked in hot path.

---

### 4. UTC Wall-Clock Timestamps

**Decision**: Use UTC wall-clock time (ISO 8601 format) for all log timestamps.

**Rationale**:
- Production debugging requires correlating logs across services
- Incident response uses UTC timestamps
- Log aggregation tools expect wall-clock time
- Monitoring systems use wall-clock time
- One syscall (`clock_gettime(CLOCK_REALTIME)`) is acceptable cost

**Output format**:
```
2026-03-27T19:45:32.123Z [INFO] app.c:42 User 42 logged in
```

**Implementation**:
- Linux: `clock_gettime(CLOCK_REALTIME, &ts)` via syscall
- Windows: `GetSystemTimePreciseAsFileTime()` or equivalent
- Format as ISO 8601 with millisecond precision

**Tradeoff**: Slightly slower than monotonic (one syscall per log), but essential for production use.

---

### 5. Leverage fun_string_template

**Decision**: Use existing `fun_string_template()` for message formatting.

**Rationale**:
- No duplicate formatting code
- Consistent with library patterns
- Already supports int, double, string, pointer types
- Tested and working

**Template syntax**:
```c
StringTemplateParam p[] = {
    {"user_id", {.intValue = 42}},
    {"ip", {.stringValue = "192.168.1.1"}}
};
log_info("User #{user_id} from ${ip} logged in", p, 2);
```

---

### 6. Console and File Outputs

**Decision**: Support two output targets: console (stdout) and file.

**Console**:
- Uses existing `fun_console_write_line()`
- Human-readable format with timestamp and level

**File**:
- Simple append-mode file writing using Fundamental's file operations
- Same format as console
- No rotation — handled by OS/log daemons (logrotate, etc.)

---

**Implementation idea**:
```
// At startup:
fun_file_open_append(log_file_path, &file_handle);

// Per log call:
fun_file_write(file_handle, formatted_message, message_length);
// File handle remains open for performance
```

**Rationale**:
- Macros can compile to nothing when disabled
- No function call overhead when disabled
- Can include `__FILE__` and `__LINE__` for debugging

---

### 8. No Structured Output (Yet)

**Decision**: Plain text output only, not JSON or logfmt.

**Rationale**:
- KISS — plain text works everywhere
- fun_transport may add structured serialization later
- Can add JSON formatter as alternative output format later

---

### 9. Source Location: Filename Only

**Decision**: Log output shows only filename (not full path) and line number.

**Rationale**:
- Prevents build environment leakage (no `/home/dev/...` paths)
- Cleaner output, easier to read
- Line number still provides precise location
- `__FILE__` always contains full path in most build systems

**Implementation**:
```c
// Macro extracts filename from __FILE__
#define LOG_BASENAME(path) \
    (strrchr(path, '/') ? strrchr(path, '/') + 1 : \
     strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path)

// Usage in log macro
#define log_info(template, params, count) \
    log_impl(LOG_LEVEL_INFO, template, params, count, \
             LOG_BASENAME(__FILE__), __LINE__)

// Output: "app.c:42" instead of "/home/dev/fundamental/src/app.c:42"
```

**Output format**:
```
2026-03-27T19:45:32.123Z [INFO] socket.c:42 User 42 logged in
```

---

## Implementation Notes

### Log Level Hierarchy

```
TRACE < DEBUG < INFO < WARN < ERROR

If FUNDAMENTAL_LOG_LEVEL = INFO:
- TRACE: compiled out
- DEBUG: compiled out
- INFO: included
- WARN: included
- ERROR: included
```

### Conditional Compilation Pattern

```c
#if FUNDAMENTAL_LOG_LEVEL <= LOG_LEVEL_INFO
    #define log_info(template, params, count) \
        log_impl(LOG_LEVEL_INFO, template, params, count, __FILE__, __LINE__)
#else
    #define log_info(template, params, count) ((void)0)
#endif
```

### Timestamp Implementation

Reuse `file_get_monotonic_ns()` from arch layer. May need to:
1. Move to a shared location (`arch/time/*/time.h`?)
2. Or create wrapper in logging module

### File Output

Simple append using Fundamental file operations:
```c
// At startup:
CanReturnError(void) result = fun_file_open_append(log_file_path, &file_handle);

// Per log call:
CanReturnError(void) result = fun_file_write(file_handle, formatted_msg, msg_length);

// File handle remains open for performance
```

Consider buffering for performance if logging is frequent.

---

## Out of Scope

- Runtime log level changes
- Async/background logging
- Log rotation
- Network output targets
- Structured (JSON) output
- Request tracing / correlation IDs
- Thread-safe concurrent logging

---

## Future Extensions

When the library evolves, these could be added:

| Extension | When |
|-----------|------|
| Wall-clock timestamps | When time API is added to platform module |
| Async logging | When threading primitives exist |
| JSON output | When fun_transport serialization exists |
| Log rotation | If OS daemons are insufficient |
| Runtime configuration | If compile-time proves too limiting |
