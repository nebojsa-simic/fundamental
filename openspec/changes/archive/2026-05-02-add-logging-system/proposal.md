## Why

The console module only provides `fun_console_print()`. Production debugging requires timestamps, log levels, file output, and async logging, forcing every project to build custom logging wrappers.

## What Changes

- Add logging macros with levels (trace, debug, info, warn, error)
- Support output targets: console and file
- Add UTC wall-clock timestamps (ISO 8601) and source location formatting
- Compile-time configuration with runtime overrides via `fun.ini`
- Leverage `fun_string_template` for message formatting
- Auto-initialize at startup via `__main()` dispatcher (zero per-call overhead)

## Capabilities

### New Capabilities
- `log-levels`: Trace, debug, info, warn, error severity levels with compile-time filtering
- `log-outputs`: Console and file output targets (synchronous)
- `log-formatting`: UTC ISO 8601 timestamp, level, file:line context formatting
- `log-config`: Runtime configuration via `fun.ini` `[logging]` section with compile-time defaults

### Removed Capabilities
- `log-async`: Deferred until threading primitives exist in the library
- `network-output`: Deferred until fun_transport exists

### Modified Capabilities
- `console`: Console module may use logging internally for structured output

## Impact

- New library: `include/fundamental/logging/`
- Zero runtime overhead when disabled (compiles to `((void)0)`)
- All configuration at compile time with `fun.ini` runtime overrides
- Enables production debugging without external dependencies
- Breaking: Requires recompilation to change log level ceiling
- Config module MUST NOT use logging (circular dependency)
