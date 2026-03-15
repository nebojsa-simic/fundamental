---
name: refactor-async-result QA status
description: QA findings for the refactor-async-result change — AsyncResult pure future, process module extraction, timeout_ms on await
type: project
---

`refactor-async-result` change verified clean on 2026-03-15. Full test suite and CLI build confirmed passing.

**What changed:**
- `AsyncResult` stripped to pure 4-field future (no `Process` field)
- `fun_async_await(result, timeout_ms)` and `fun_async_await_all(results, count, timeout_ms)` now take `int timeout_ms`; both return `CanReturnError(void)`
- Process spawn extracted to its own module: `include/process/`, `src/process/`, `arch/process/{windows,linux}-amd64/`
- Old `arch/async/*/process.c` and `src/async/process.c` deleted; `tests/process_spawn/` deleted; `tests/process/` is the new home
- All `fun_async_await` call sites across all tests and `fundamental-cli` updated to pass `timeout_ms = -1`

**Verification results (2026-03-15):**
- Full suite (`run-tests-windows-amd64.bat`): all test modules pass, zero failures
- New process module tests: 7/7 pass (`test_process_spawn_success`, `test_process_spawn_not_found`, `test_process_stdout_capture`, `test_process_stderr_capture`, `test_process_exit_code`, `test_process_terminate`, `test_process_buffer_truncation`)
- New async tests including timeout: 7/7 pass (including `test_fun_async_await_timeout_zero`, `test_fun_async_await_timeout_expires`)
- `fundamental-cli` build: clean (`Build complete: fun.exe`)
- `fun.exe version`: outputs correctly, no crash
- `fun.exe test`: returns "No tests found" gracefully

**Old API scan result:** Zero references to `fun_async_process_spawn`, `fun_process_get_exit_code`, `fun_process_get_stdout`, `fun_process_get_stderr`, or `result.process.` anywhere in src/, include/, tests/, arch/ (excluding vendor/).

**Leftover file check:**
- `arch/async/linux-amd64/` and `arch/async/windows-amd64/` exist as empty directories — no .c files, no compile impact. Cosmetic only; can be deleted if desired.
- `src/async/process.c`: deleted, confirmed absent.
- `tests/process_spawn/`: deleted, confirmed absent.

**Warnings in this run:** All pre-existing (see project_config_module.md). No new warnings introduced by this change.

**Why:** Architectural separation of concerns — async is now a pure future/await mechanism; process spawn is its own module with clean API.
**How to apply:** When checking process-related code, look in `include/process/process.h`, `src/process/`, and `arch/process/`. Async tests are in `tests/async/`, process tests in `tests/process/`.
