## Context

The Fundamental library provides memory, string, file, and async operations but lacks console output capabilities. CLI applications require stdout/stderr output, which currently would need stdlib functions (printf, puts, fprintf) violating the zero-stdlib-runtime design principle. This design adds console I/O with line buffering following existing library patterns.

Current state: No console output module. String module handles string operations but has no I/O capabilities. File module handles file I/O but not console streams.

## Goals / Non-Goals

**Goals:**
- Provide fun_console_write_line() for stdout output with automatic newline
- Provide fun_console_error_line() for stderr output with automatic newline
- Provide fun_console_write() for raw output without newline
- Implement line buffering (512 bytes) for efficient output
- Platform abstraction (Windows Console API, POSIX write())
- Zero stdlib runtime functions (headers OK for types, no runtime calls)
- Follow existing error handling patterns with ErrorResult

**Non-Goals:**
- Console input reading (stdin) - defer to future change
- Colored/formatted output - can be added later with ANSI escape codes
- Interactive prompts or progress bars - higher-level CLI features
- Configurable buffer sizes - keep simple for now
- Thread safety - single-threaded CLI usage initially

## Decisions

### 1. Function Naming and API
**Decision:** `fun_console_write_line()`, `fun_console_error_line()`, `fun_console_write()`

**Rationale:** Clear intent, consistent with fundamental naming conventions. Separate functions for stdout vs stderr instead of enum parameter - simpler call sites, no runtime dispatch.

**Alternatives Considered:**
- `fun_io_write(IoStream stream, ...)` - More flexible but adds enum complexity
- `fun_stdout_write()`, `fun_stderr_write()` - More verbose, less discoverable

### 2. Line Buffering Strategy
**Decision:** 512-byte static buffer, auto-flush on newline or buffer full

**Rationale:** Reduces syscall count for small writes. 512 bytes balances memory usage vs efficiency. Static allocation avoids dynamic memory management complexity.

**Alternatives Considered:**
- No buffering (direct writes) - Simpler but inefficient for many small writes
- Dynamic buffer - More flexible but requires memory management
- Configurable size - Adds complexity without clear benefit for CLI use case

### 3. String-Only Interface
**Decision:** Accept `String` (const char*) only, no raw buffer API

**Rationale:** Type-safe, consistent with fundamental patterns. CLI output is predominantly string-based. Raw buffer writes can be added later if needed.

**Alternatives Considered:**
- Also provide `fun_console_write_raw(const void*, size_t)` - More flexible but adds API surface
- Only raw buffer API - Forces callers to manage lengths, less convenient

### 4. Platform Abstraction
**Decision:** Separate platform files (arch/console/windows-amd64/, arch/console/linux-amd64/)

**Rationale:** Matches existing library structure. Clear separation of platform-specific code. Easy to add more platforms later.

**Alternatives Considered:**
- Single file with #ifdef - Harder to maintain, more complex
- Runtime function pointers - Overkill for this use case

### 5. Error Handling
**Decision:** Return ErrorResult, use existing error codes or add CONSOLE_WRITE_FAILED

**Rationale:** Consistent with library conventions. Callers already understand error checking pattern.

**Alternatives Considered:**
- Return void and log errors internally - Hides failures from callers
- Dedicated console error type - Adds API surface without clear benefit

## Risks / Trade-offs

**[Buffer flush timing]** → Auto-flush on newline means partial lines may not appear immediately. Mitigation: Document behavior, provide fun_console_flush() for explicit control.

**[Static buffer in multithreaded code]** → Single static buffer not thread-safe. Mitigation: Document single-threaded usage, add thread-local storage if needed later.

**[Windows console encoding]** → Windows uses UTF-16, POSIX uses UTF-8. For now, use ANSI API (WriteFile) with UTF-8. Mitigation: Can add wide char support later if Unicode needed.

**[Buffer overflow on very long lines]** → Lines > 512 bytes need special handling. Mitigation: Flush buffer and write directly for oversized strings.

**[Performance vs simplicity]** → Static buffer simpler than dynamic but limits concurrent output streams. Trade-off acceptable for CLI use case.

## Migration Plan

Not applicable - this is a new capability with no existing API to migrate.

## Open Questions

- Should we add fun_console_flush() in phase 1 or defer?
- Is 512 bytes the right default buffer size?
- Should we support Windows Unicode (UTF-16) from the start?
- Any demand for colored output (ANSI escape codes) in phase 1?
