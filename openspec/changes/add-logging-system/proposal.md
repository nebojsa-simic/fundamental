## Why

The console module only provides `fun_console_print()`. Production debugging requires timestamps, log levels, file output, and async logging, forcing every project to build custom logging wrappers.

## What Changes

- Add structured logging with levels (trace, debug, info, warn, error)
- Support multiple output targets (console, file, network)
- Add timestamps and structured fields
- Provide async logging for non-blocking operation

## Capabilities

### New Capabilities
- `log-levels`: Trace, debug, info, warn, error severity levels
- `log-outputs`: Configurable output targets (console, file, socket)
- `log-formatting`: Timestamp, level, context formatting
- `log-async`: Non-blocking async log writer with ring buffer

### Modified Capabilities
- `console`: Console module may use logging for structured output

## Impact

- New library: `include/fundamental/logging/`
- Minimal runtime overhead when logging disabled
- Configurable at compile time and runtime
- Enables production observability without external dependencies
