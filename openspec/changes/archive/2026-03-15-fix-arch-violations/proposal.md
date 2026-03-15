## Why

Two modules (`async` and `stream`) contain OS-specific logic directly in `src/`, violating the principle that all platform-specific code must live exclusively in `arch/`. This creates hidden portability risks and undermines the architectural guarantee that `src/` is platform-neutral.

## What Changes

- Move platform-specific time implementation out of `src/async/async.c` (`#ifdef _WIN32` / `clock_gettime` block) into `arch/async/windows-amd64/async.c` and `arch/async/linux-amd64/async.c`, exposing a clean `arch_async_now_ms()` function
- Refactor `src/stream/streamLifecycle.c` to remove direct use of Windows types (`INVALID_HANDLE_VALUE`, `CloseHandle()`) and direct inclusion of the Windows arch header; move that logic into the appropriate arch layer

## Capabilities

### New Capabilities

- `async-arch`: Platform time abstraction for the async module — `arch_async_now_ms()` implemented per arch layer, called from `src/async/async.c`
- `stream-arch`: Platform stream lifecycle abstraction — open/close handle logic moved to arch layer, called from `src/stream/streamLifecycle.c`

### Modified Capabilities

- `async`: Internal implementation change only — no API or behaviour changes
- `stream`: Internal implementation change only — no API or behaviour changes

## Impact

- `src/async/async.c` — remove `#ifdef _WIN32` block, call `arch_async_now_ms()` instead
- `arch/async/windows-amd64/async.c` — new file implementing `arch_async_now_ms()` via `GetTickCount64()`
- `arch/async/linux-amd64/async.c` — new file implementing `arch_async_now_ms()` via `clock_gettime()`
- `src/stream/streamLifecycle.c` — remove Windows arch header include and `CloseHandle()`/`INVALID_HANDLE_VALUE` usage
- `arch/stream/windows-amd64/` — extend with lifecycle functions
- `arch/stream/linux-amd64/` — extend with lifecycle functions
- No public API changes; no downstream consumers affected
