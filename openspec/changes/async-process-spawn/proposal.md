## Why

The library currently lacks the ability to spawn and manage external processes asynchronously. Users need to execute command-line tools, scripts, or other executables without blocking the main thread, while capturing stdout/stderr output for processing. This change adds process spawning capabilities inspired by Golang's `os/exec` package.

## What Changes

- New async process spawning API in the `async` module
- Process handle management with start, wait, and terminate operations
- Stdout/stderr stream capture with configurable buffering
- Process exit code retrieval and status reporting
- Cross-platform process execution (Windows handles, POSIX compatibility)

## Capabilities

### New Capabilities
- `process-spawn`: Async process spawning with stdout/stderr capture, exit code retrieval, and process lifecycle management

### Modified Capabilities
- `async`: Extended to support process-related async operations (new result types and status handling)

## Impact

- New public API in `include/fundamental/async.h` for process operations
- New source files in `src/async/process.c` for implementation
- Platform-specific code in `arch/*/process.c` for Windows and POSIX
- New test suite in `tests/process_spawn/` for validation
- No breaking changes to existing APIs
