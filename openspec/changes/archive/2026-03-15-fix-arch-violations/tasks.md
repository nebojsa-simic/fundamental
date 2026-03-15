## 1. Async — Move time abstraction to arch layer

- [x] 1.1 Create `arch/async/windows-amd64/async.c` implementing `arch_async_now_ms()` using `GetTickCount64()`
- [x] 1.2 Create `arch/async/linux-amd64/async.c` implementing `arch_async_now_ms()` using `clock_gettime(CLOCK_MONOTONIC)`
- [x] 1.3 Remove the `#ifdef _WIN32` block from `src/async/async.c` and add an `extern unsigned long long arch_async_now_ms(void);` declaration in its place

## 2. Stream — Move handle-close to arch layer

- [x] 2.1 Add `arch_stream_close_handle(void *internal_state)` to `arch/stream/windows-amd64/streamOpen.c` (calls `CloseHandle` if handle is not `INVALID_HANDLE_VALUE`)
- [x] 2.2 Add `arch_stream_close_handle(void *internal_state)` to `arch/stream/linux-amd64/streamOpen.c` (issues `close` syscall if fd is not `-1`)
- [x] 2.3 Update `src/stream/streamLifecycle.c`: remove `#include "../arch/stream/windows-amd64/stream.h"`, add `extern void arch_stream_close_handle(void *internal_state);`, replace `CloseHandle` / `INVALID_HANDLE_VALUE` logic with call to `arch_stream_close_handle(state)`

## 3. Build script updates — add new arch/async files

- [x] 3.1 Update `tests/async/build-windows-amd64.bat` — add `arch/async/windows-amd64/async.c`
- [x] 3.2 Update `tests/async/build-linux-amd64.sh` — add `arch/async/linux-amd64/async.c`
- [x] 3.3 Update `tests/stream/build-windows-amd64.bat` — add `arch/async/windows-amd64/async.c`
- [x] 3.4 Update `tests/stream/build-linux-amd64.sh` — add `arch/async/linux-amd64/async.c`
- [x] 3.5 Update `tests/stream/build-windows-amd64-canWrite.bat` — add `arch/async/windows-amd64/async.c`
- [x] 3.6 Update `tests/stream/build-windows-amd64-streamWrite.bat` — add `arch/async/windows-amd64/async.c`
- [x] 3.7 Update all remaining test build scripts that include `src/async/async.c` — add the matching `arch/async/<platform>/async.c` (collections, fileAppend, fileLock, fileRead, fileWrite, hashmap, memory, network, process, rbtree, set)
- [x] 3.8 Update `run-tests-windows-amd64.bat` and `run-tests-linux-amd64.sh` if they compile async directly

## 4. Verification

- [x] 4.1 Run `code-format.bat` to format all modified and new files
- [x] 4.2 Build and run the async test suite on Windows (`tests/async/build-windows-amd64.bat`)
- [x] 4.3 Build and run the stream test suite on Windows (`tests/stream/build-windows-amd64.bat`)
- [x] 4.4 Confirm `src/async/async.c` contains no `#ifdef`, no `<windows.h>`, no `<time.h>`
- [x] 4.5 Confirm `src/stream/streamLifecycle.c` contains no arch/ header path and no OS handle types
