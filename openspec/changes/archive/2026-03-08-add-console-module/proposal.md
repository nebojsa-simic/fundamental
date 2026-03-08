## Why

The Fundamental library lacks console output capabilities for CLI applications, requiring stdlib functions (printf, puts) that violate the zero-stdlib-runtime design principle. This change adds console I/O with line buffering to enable `fundamental-cli` and other applications without external dependencies.

## What Changes

- New console module with line-buffered output functions
- Platform-abstracted stdout/stderr writes (Windows Console API, POSIX write())
- No stdlib runtime functions used in implementation
- No breaking changes to existing APIs

## Capabilities

### New Capabilities
- `console`: Line-buffered console output for stdout and stderr streams with automatic newline handling

### Modified Capabilities
- None

## Impact

- New public API in `include/console/console.h` with fun_console_* functions
- New source files in `src/console/console.c` for buffering logic
- Platform-specific code in `arch/console/windows-amd64/` and `arch/console/linux-amd64/`
- Enables CLI development without stdlib runtime dependencies
- No breaking changes to existing modules
