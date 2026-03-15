## Context

`AsyncResult` was designed as a generic future but grew a `Process` field to carry process spawn output, and `fun_async_process_spawn` lived inside the `async` module. This inverted the dependency: `async` knew about process spawning, when it should know nothing about specific operations. `fun_async_await` also blocked indefinitely with no timeout.

The fix establishes a clean module hierarchy: `async` is a pure future primitive; `process` is a new first-class module that depends on `async` and owns all process spawning concerns.

## Goals / Non-Goals

**Goals:**
- `async` module contains only `AsyncResult`, `AsyncStatus`, `AsyncPollFn`, `fun_async_await`, `fun_async_await_all` — nothing else, ever
- `process` module is a complete, self-contained module following the standard module structure (`include/`, `src/`, `arch/`, `tests/`)
- `fun_async_await` / `fun_async_await_all` return `CanReturnError(void)` with `timeout_ms`; on timeout `result->status = ASYNC_ERROR`
- Caller controls process output buffer sizes via pointer+capacity fields on `ProcessResult`
- Direct field access on `ProcessResult` replaces all `fun_process_get_*` accessor functions

**Non-Goals:**
- Adding cancellation support beyond timeout
- Changing the poll-based mechanics of `AsyncResult`
- Making `fun_async_await` thread-safe
- Moving any other module's logic out of `async` (only process is affected)

## Decisions

### D1: Module dependency direction

```
      depends on
process ──────────▶ async
network ──────────▶ async   (future)
file    ──────────▶ async   (future)

async   ──────────▶ (nothing — pure primitive)
```

`async` has zero knowledge of any specific operation. Each operation module returns `AsyncResult` from its spawn/connect/read functions and includes `async/async.h`.

**Alternative considered**: Keep process in `async`, just remove `Process` from the struct. Rejected — the functions `fun_async_process_spawn`, `fun_process_terminate`, `fun_process_free` still conceptually pollute the async module regardless of where the struct lives. A full module split is the only clean answer.

### D2: `AsyncResult` stripped to pure future

```c
struct AsyncResult {
    AsyncPollFn  poll;    /* how to advance this operation */
    void        *state;   /* opaque per-operation state    */
    AsyncStatus  status;  /* PENDING / COMPLETED / ERROR   */
    ErrorResult  error;   /* set when status == ASYNC_ERROR */
};
```

`state` is a `void *` — each module's arch layer stores its OS handle there. Callers never touch `state` directly.

### D3: New `process` module structure

```
include/process/process.h          ← public API
src/process/process.c              ← arg validation, arch delegation
arch/process/windows-amd64/process.c
arch/process/linux-amd64/process.c
tests/process/test_process.c       ← renamed from tests/process_spawn/
tests/process/build-windows-amd64.bat
tests/process/build-linux-amd64.sh
```

Arch code moves from `arch/async/*/async.c` into `arch/process/*/process.c`. The `arch/async/` layer shrinks to only the timeout clock implementation (if needed).

### D4: `ProcessResult` as caller-allocated output

```c
typedef struct {
    void   *_handle;        /* opaque OS handle; for terminate/free only */
    char   *stdout_data;    /* caller-provided buffer                    */
    size_t  stdout_capacity;
    size_t  stdout_length;  /* filled by library on completion           */
    char   *stderr_data;    /* caller-provided buffer                    */
    size_t  stderr_capacity;
    size_t  stderr_length;  /* filled by library on completion           */
    int     exit_code;      /* filled by library on completion           */
} ProcessResult;
```

Caller sets `stdout_data`/`stdout_capacity` and `stderr_data`/`stderr_capacity` before calling `fun_process_spawn`. Library fills `_handle` immediately (needed for terminate before await), and fills lengths and exit code on completion.

**Alternative considered**: Fixed embedded buffers inside `ProcessResult`. Rejected — matches the `NetworkBuffer` pattern; caller controls allocation; no hidden size constants in structs.

### D5: `fun_process_spawn` signature

```c
AsyncResult fun_process_spawn(
    const char                *executable,
    const char               **args,
    const ProcessSpawnOptions *options,
    ProcessResult             *out
);
```

Renamed from `fun_async_process_spawn` — the `async_` prefix was a symptom of the old coupling. Naming now matches every other module: `fun_process_spawn`, `fun_process_terminate`, `fun_process_free`.

### D6: Await timeout semantics

```c
CanReturnError(void) fun_async_await(AsyncResult *result, int timeout_ms);
CanReturnError(void) fun_async_await_all(AsyncResult **results, size_t count, int timeout_ms);
```

`timeout_ms` convention: `-1` = block indefinitely, `0` = single poll, `> 0` = deadline in ms.

On timeout: `result->status` set to `ASYNC_ERROR`, `result->error` set to `ERROR_RESULT_ASYNC_TIMEOUT`, function returns that error. `ASYNC_ERROR` is terminal — no ambiguous pending state for the caller to reason about.

For `fun_async_await_all`: timeout is wall-clock across all results. Any result still pending at deadline is set to `ASYNC_ERROR`. Returns on first timeout error.

### D7: Remove all `fun_process_get_*` accessors

`fun_process_get_stdout`, `fun_process_get_stderr`, `fun_process_get_exit_code`, `fun_async_result_get_process` are removed entirely. Direct field access:

```c
/* before */
const char *out = fun_process_get_stdout(&result, &len);

/* after */
proc.stdout_data[0 .. proc.stdout_length]
```

`fun_process_terminate(ProcessResult *)` and `fun_process_free(ProcessResult *)` remain — they need `_handle`.

## Risks / Trade-offs

**Breaking change to all callers** → All call sites are within this repo. Tests and fundamental-cli updated as part of this change.

**`arch/async/` may become empty** → If the only thing left in `arch/async/` after removing process code is a clock helper for timeout, consider whether it belongs in `arch/async/` or `arch/platform/`. Leave it in `arch/async/` for now; a future platform-clock extraction is a separate concern.

**Tight poll loop** → `fun_async_await` calls `poll` in a loop. For future network async ops, `poll` must call `run_once` with a blocking wait to avoid CPU spin. This is each module's arch-layer responsibility.

## Migration Plan

1. Add `ERROR_CODE_ASYNC_TIMEOUT` to `include/error/error.h`
2. Strip `include/async/async.h` to pure future; update `fun_async_await` / `fun_async_await_all` signatures
3. Update `src/async/async.c` with timeout logic; remove process code
4. Create `include/process/process.h` with `ProcessResult`, `ProcessSpawnOptions`, `fun_process_spawn`, `fun_process_terminate`, `fun_process_free`
5. Create `src/process/process.c` — arg validation, arch delegation
6. Move arch process code into `arch/process/windows-amd64/process.c` and `arch/process/linux-amd64/process.c`
7. Create `tests/process/` (rename from `tests/process_spawn/`); update all call sites
8. Update `tests/async/` for new await signature and add timeout tests
9. Update `fundamental-cli/` — spawn call sites, build scripts
10. Update `CLAUDE.md` module table; update `fundamental-async.md` and `fundamental-async.md` skills
11. Run full test suite
