## Context

Two `src/` files contain OS-specific logic, violating the invariant that `src/` is always platform-neutral:

1. `src/async/async.c` — defines `async_now_ms()` via a `#ifdef _WIN32` block, with `<windows.h>` / `GetTickCount64()` on Windows and `<time.h>` / `clock_gettime()` on Linux
2. `src/stream/streamLifecycle.c` — hard-codes `#include "../arch/stream/windows-amd64/stream.h"` and directly calls `CloseHandle()` / tests `INVALID_HANDLE_VALUE`

The rest of the codebase resolves this cleanly in two ways:
- **`extern` forward-declaration pattern** (console.c) — `src/` file declares arch functions with `extern` and the linker resolves them from the arch object
- **Named forward-declaration block** (network.c) — same idea, more explicit documentation

Neither arch module (`arch/async/`, `arch/stream/`) currently exists for async.

## Goals / Non-Goals

**Goals:**
- Remove all `#ifdef` / OS-specific includes from `src/async/async.c`
- Remove Windows arch header include and `CloseHandle()` / `INVALID_HANDLE_VALUE` from `src/stream/streamLifecycle.c`
- Add platform implementations in `arch/async/` and extend `arch/stream/` for both Windows and Linux

**Non-Goals:**
- No changes to any public API (`fun_async_await`, `fun_async_await_all`, `fun_stream_close`)
- No changes to test files
- No changes to build scripts beyond adding the new arch source files

## Decisions

### D1: Use `extern` forward-declarations in `src/` (not a shared arch header)

Following the console/network pattern, `src/` files will `extern`-declare the arch function signatures they need. No new header files required in `arch/`.

**Alternative considered**: A shared `arch/async/async_arch.h` header included by `src/`. Rejected — adds a file, and the existing codebase convention is `extern` in the `.c` file itself.

### D2: `arch_async_now_ms()` — signature `unsigned long long arch_async_now_ms(void)`

Matches the exact usage in `src/async/async.c`. The `unsigned long long` type is already used in the file; no new types needed.

### D3: `arch_stream_close_handle(void *internal_state)` — opaque pointer parameter

`src/stream/streamLifecycle.c` casts `stream->internal_state` to `StreamReadState *` only to call `CloseHandle`. By taking a `void *`, the arch function receives the opaque state and handles the OS-specific part (close the handle) internally. Memory freeing stays in `src/` with `fun_memory_free()` as today, because that is platform-neutral.

**Alternative considered**: Moving all of `fun_stream_close` into the arch layer. Rejected — `fun_memory_free` is platform-neutral and belongs in `src/`. Only the handle-close is platform-specific.

### D4: New `arch/async/` directory structure

Create `arch/async/windows-amd64/async.c` and `arch/async/linux-amd64/async.c`. The async module had no arch layer previously because the original implementation chose `#ifdef` as a shortcut. Build scripts must be updated to compile these new files.

## Risks / Trade-offs

- **Build script updates required** — every build script that compiles `src/async/async.c` must also add the corresponding arch file. Missing one causes a linker error on `arch_async_now_ms`. Mitigation: update all build scripts as part of this change and run the full test suite.
- **`arch_stream_close_handle` with `void *`** — loses type safety at the call site. Acceptable because this is an internal arch boundary, not a public API. The arch implementation immediately casts to the concrete arch type it owns.
