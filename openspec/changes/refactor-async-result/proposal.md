## Why

`AsyncResult` embeds an 8KB `Process` struct unconditionally, making it useless as a generic future type — every file read, network op, or other async operation would carry process baggage. Process spawning belongs in its own module, with `async` as a pure future primitive that other modules depend on. The `fun_async_await` function also provides no timeout, forcing callers to block indefinitely or build their own polling loops.

## What Changes

- **BREAKING** Remove `Process` field from `AsyncResult` — it becomes a pure 4-field future handle (`poll`, `state`, `status`, `error`)
- **BREAKING** `fun_async_await` and `fun_async_await_all` gain a `timeout_ms` parameter and return `CanReturnError(void)` instead of `void`; on timeout the result's `status` is set to `ASYNC_ERROR` and `ERROR_CODE_ASYNC_TIMEOUT` is returned
- **BREAKING** All process spawn and process management functions move out of `async` into a new `process` module (`include/process/process.h`, `src/process/`, `arch/process/*/`)
- **BREAKING** `fun_async_process_spawn` renamed to `fun_process_spawn`; gains a `ProcessResult *out` parameter; caller allocates and owns the output buffer
- **BREAKING** `fun_process_terminate` and `fun_process_free` move to `process` module and take `ProcessResult *`
- Remove `fun_async_result_get_process`, `fun_process_get_stdout`, `fun_process_get_stderr`, `fun_process_get_exit_code` — replaced by direct field access on `ProcessResult`
- Add `ERROR_CODE_ASYNC_TIMEOUT` to `error.h`
- Introduce `ProcessResult` as a caller-allocated output type in the `process` module with pointer+capacity+length fields for stdout and stderr

## Capabilities

### New Capabilities

- `async-await`: Updated await contract — timeout parameter, `CanReturnError(void)` return, `ASYNC_ERROR` status on timeout
- `process`: Dedicated process module — `fun_process_spawn` (returns `AsyncResult`), `fun_process_terminate`, `fun_process_free`, `ProcessResult` caller-allocated output type

### Modified Capabilities

- `async`: `AsyncResult` struct stripped to pure future (`poll`, `state`, `status`, `error`); `fun_async_await` / `fun_async_await_all` signatures change; all process functions removed

## Impact

- `include/async/async.h` — stripped to pure future types and await functions only
- `include/process/process.h` — new file: `ProcessResult`, `ProcessSpawnOptions`, `fun_process_spawn`, `fun_process_terminate`, `fun_process_free`
- `src/process/process.c` — new file: arg validation, delegates to arch layer
- `arch/process/windows-amd64/process.c` — new file: Windows process spawn implementation (moved from `arch/async/`)
- `arch/process/linux-amd64/process.c` — new file: Linux process spawn implementation (moved from `arch/async/`)
- `src/async/async.c` — poll loop gains timeout logic; process code removed
- `arch/async/*/async.c` — process spawn code removed
- `include/error/error.h` — new `ERROR_CODE_ASYNC_TIMEOUT` constant and static result
- `tests/process_spawn/` — renamed to `tests/process/`; updated for new module and signatures
- `tests/async/` — await signature updates and new timeout tests
- `fundamental-cli/` — spawn call sites updated; build scripts gain `process` module sources
- `CLAUDE.md` — new `Process` module row; `Async` row updated
