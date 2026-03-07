## Context

The Fundamental library provides async operations for I/O but lacks process execution capabilities. Users need to spawn external processes (compilers, scripts, tools) and capture their output asynchronously without blocking the event loop. This design adds process spawning following the library's existing async patterns and memory management conventions.

Current architecture uses `AsyncResult`/`AsyncStatus` pattern for non-blocking operations. Process spawning will follow this same pattern for consistency.

## Goals / Non-Goals

**Goals:**
- Async process spawn returning AsyncResult for use with fun_async_await
- Capture stdout/stderr streams with configurable buffering
- Cross-platform support (Windows CreateProcess, POSIX fork/exec)
- Process lifecycle management (start, terminate, exit code)
- Follow existing async and memory management patterns
- Zero-copy stream access where possible

**Non-Goals:**
- Process pools or worker management
- Interactive stdin piping (read-only stdout/stderr initially)
- Process groups or sessions
- Real-time output streaming callbacks
- Environment variable inheritance control

## Decisions

### 1. Process Handle Structure
**Decision:** `Process` struct embedded inside `AsyncResult` as opaque data. Process handle accessed via `fun_async_result_get_process(&result)`.

**Rationale:** Eliminates separate Process parameter. AsyncResult owns the process lifecycle. Caller only manages AsyncResult, not separate handle.

**Alternatives Considered:**
- Separate Process parameter: More verbose API, risk of mismatched handles
- Process contains AsyncResult: Backwards, AsyncResult should own operation state

### 2. Stream Capture Strategy
**Decision:** Fixed-size circular buffers for stdout/stderr, configurable at spawn time.

**Rationale:** Prevents unbounded memory growth. Allows caller to read incrementally. Zero-copy access via pointer + length.

**Alternatives Considered:**
- Dynamic growth: Simpler API but risk of memory exhaustion
- File-backed pipes: Better for large output but slower and cleanup complexity
- Callback-based streaming: More complex, harder to compose with other async ops

### 3. Async Result as Process Handle
**Decision:** `AsyncResult` contains embedded `Process` data. All process operations take `AsyncResult*` not separate `Process*`.

**Rationale:** Single handle to manage. Process lifecycle tied to AsyncResult lifecycle. No separate allocation or parameter passing.

**API Example:**
```c
AsyncResult result = fun_async_process_spawn("/bin/ls", args, &options);
fun_async_await(&result);                    // Wait using existing async
const char *stdout = fun_process_get_stdout(&result);
int exit_code = fun_process_get_exit_code(&result);
fun_process_free(&result);                   // Cleanup
```

**Alternatives Considered:**
- Separate Process handle: Requires managing two objects, API verbosity
- Process embeds AsyncResult: Wrong ownership model

### 4. Error Handling
**Decision:** Process errors use existing error code system (spawn failure, not found, permission denied).

**Rationale:** Consistent with library conventions. Callers already understand error checking pattern.

**Alternatives Considered:**
- Dedicated process error type: More specific but adds API surface
- errno passthrough: Platform-specific, breaks abstraction

## Risks / Trade-offs

**[Buffer overflow]** → Fixed circular buffer drops oldest data when full. Documented behavior, caller can size appropriately.

**[Zombie processes]** → Platform-specific cleanup in wait/terminate. Windows handles auto-cleanup, POSIX needs waitpid.

**[Deadlock on full buffer]** → If caller never reads and buffer fills, process may block on write. Mitigation: document buffer sizing, consider non-blocking pipe option.

**[Windows path handling]** → Windows requires special path handling for executables. Use `_waccess` and wide char APIs where needed.

**[Signal handling on POSIX]** → fork/exec needs careful signal mask handling. Defer to platform layer, document expected behavior.

## Migration Plan

Not applicable - this is a new capability with no existing API to migrate.

## Open Questions

- Should we support stdin piping in phase 2?
- Is there demand for process groups/sessions?
- Should buffer size default be configurable globally or per-process only?
