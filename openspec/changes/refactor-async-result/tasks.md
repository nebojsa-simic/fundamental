## 1. Error Codes

- [x] 1.1 Add `ERROR_CODE_ASYNC_TIMEOUT` constant to `include/error/error.h`
- [x] 1.2 Add `ERROR_RESULT_ASYNC_TIMEOUT` static variable to `include/error/error.h`

## 2. Strip async Module to Pure Future

- [x] 2.1 Remove `Process`, `ProcessSpawnOptions` types and all `fun_async_process_spawn` / `fun_process_*` declarations from `include/async/async.h`
- [x] 2.2 Remove `Process` field from `AsyncResult` struct — leave only `poll`, `state`, `status`, `error`
- [x] 2.3 Update `fun_async_await` signature: add `int timeout_ms`, change return type to `CanReturnError(void)`
- [x] 2.4 Update `fun_async_await_all` signature: add `int timeout_ms`, change return type to `CanReturnError(void)`
- [x] 2.5 Update `src/async/async.c`: implement timeout loop (`-1` = indefinite, `0` = single poll, `> 0` = deadline in ms); set `result->status = ASYNC_ERROR` and `result->error = ERROR_RESULT_ASYNC_TIMEOUT` on expiry; return that error
- [x] 2.6 Update `fun_async_await_all` in `src/async/async.c`: apply same timeout logic; set `ASYNC_ERROR` on all still-pending results at deadline; return first timeout error
- [x] 2.7 Remove all process spawn code from `arch/async/windows-amd64/async.c`
- [x] 2.8 Remove all process spawn code from `arch/async/linux-amd64/async.c`

## 3. Create process Module Header

- [x] 3.1 Create `include/process/process.h` with `ProcessResult` struct: `_handle`, `stdout_data`, `stdout_capacity`, `stdout_length`, `stderr_data`, `stderr_capacity`, `stderr_length`, `exit_code`
- [x] 3.2 Add `ProcessSpawnOptions` struct to `include/process/process.h` (moved from async)
- [x] 3.3 Declare `fun_process_spawn(executable, args, options, ProcessResult *out)` returning `AsyncResult`
- [x] 3.4 Declare `fun_process_terminate(ProcessResult *out)` returning `CanReturnError(void)`
- [x] 3.5 Declare `fun_process_free(ProcessResult *out)` returning `CanReturnError(void)`

## 4. Create process Module Core (src/process/)

- [x] 4.1 Create `src/process/process.c` — validate args, delegate to `fun_process_arch_spawn` / `fun_process_arch_terminate` / `fun_process_arch_free`

## 5. Create process Windows Arch Layer

- [x] 5.1 Create `arch/process/windows-amd64/process.c` — move Windows process spawn implementation from `arch/async/windows-amd64/async.c`; write OS handles into `ProcessResult->_handle`; write stdout/stderr into caller buffers respecting capacities; fill lengths and exit code on completion
- [x] 5.2 Implement `fun_process_arch_terminate` for Windows: read OS handle from `ProcessResult->_handle`, call `TerminateProcess`
- [x] 5.3 Implement `fun_process_arch_free` for Windows: close process and pipe handles in `ProcessResult->_handle`

## 6. Create process Linux Arch Layer

- [x] 6.1 Create `arch/process/linux-amd64/process.c` — move Linux process spawn implementation from `arch/async/linux-amd64/async.c`; same output contract as Windows
- [x] 6.2 Implement `fun_process_arch_terminate` for Linux: read OS handle from `ProcessResult->_handle`, call `kill(SIGTERM)`
- [x] 6.3 Implement `fun_process_arch_free` for Linux: close pipe fds, `waitpid` to reap zombie if needed

## 7. Create process Tests

- [x] 7.1 Create `tests/process/` directory with build scripts (copy and update from `tests/process_spawn/`)
- [x] 7.2 Create `tests/process/test_process.c` — update all `fun_async_process_spawn` → `fun_process_spawn`, add `ProcessResult` with buffer pointers, pass `&proc` as last arg; include `process/process.h` instead of `async/async.h`
- [x] 7.3 Replace `fun_process_get_stdout(&result, &len)` → `proc.stdout_data` / `proc.stdout_length` direct access
- [x] 7.4 Replace `fun_process_get_stderr(&result, &len)` → `proc.stderr_data` / `proc.stderr_length` direct access
- [x] 7.5 Replace `fun_process_get_exit_code(&result)` → `proc.exit_code` direct access
- [x] 7.6 Update `fun_process_terminate` and `fun_process_free` call sites to pass `&proc`
- [x] 7.7 Update all `fun_async_await` call sites: add `timeout_ms = -1`, handle `voidResult` return
- [x] 7.8 Set execute bit on `tests/process/build-linux-amd64.sh` with `git update-index --chmod=+x`

## 8. Update async Tests

- [x] 8.1 Update all `fun_async_await` call sites in `tests/async/test.c`: add `timeout_ms = -1`, handle `voidResult` return
- [x] 8.2 Update all `fun_async_await_all` call sites in `tests/async/test.c`
- [x] 8.3 Add test: `fun_async_await` with `timeout_ms = 0` on incomplete op returns `ERROR_CODE_ASYNC_TIMEOUT`, sets `ASYNC_ERROR`
- [x] 8.4 Add test: `fun_async_await` with `timeout_ms > 0` that expires sets `ASYNC_ERROR` with `ERROR_CODE_ASYNC_TIMEOUT`

## 9. Update fundamental-cli

- [x] 9.1 Add `#include "process/process.h"` and update `src/commands/cmd_clean.c`: declare `ProcessResult`, initialise buffer pointers, use `fun_process_spawn`, `fun_process_free(&proc)`
- [x] 9.2 Update `src/test/runner.c`: declare `ProcessResult`, initialise buffer pointers, use `fun_process_spawn`, replace `fun_process_get_exit_code` with `proc.exit_code`
- [x] 9.3 Update any other `fun_async_await` call sites in fundamental-cli for new signature
- [x] 9.4 Add `vendor/fundamental/src/process/process.c` and `vendor/fundamental/arch/process/windows-amd64/process.c` to `build-windows-amd64.bat`
- [x] 9.5 Add `vendor/fundamental/src/process/process.c` and `vendor/fundamental/arch/process/linux-amd64/process.c` to `build-linux-amd64.sh`

## 10. Documentation and Skills

- [x] 10.1 Update `.opencode/skills/fundamental-async.md` — remove process spawn examples; add timeout parameter to all await examples
- [x] 10.2 Create `.opencode/skills/fundamental-process.md` — Quick Reference + copy-paste examples for spawn, terminate, free, direct field access
- [x] 10.3 Add `process` row to skills index in `.opencode/skills/fundamental-skills-index.md`
- [x] 10.4 Update `CLAUDE.md`: add `Process` module row; update `Async` row to reflect stripped-down scope; add `process` to test modules list
- [x] 10.5 Run `vendor-fundamental.bat` in `fundamental-cli/` to sync vendored sources

## 11. Code Formatting and Verification

- [x] 11.1 Run `code-format.bat` on all modified and new `.c` and `.h` files
- [x] 11.2 Build and run `tests/async/` — all tests pass
- [x] 11.3 Build and run `tests/process/` — all tests pass
- [x] 11.4 Run `run-tests-windows-amd64.bat` — full suite passes
- [x] 11.5 Build `fundamental-cli` with `build-windows-amd64.bat` and run the CLI smoke test — binary launches, executes a command that spawns a process (e.g. `fundamental-cli test`), exits cleanly
