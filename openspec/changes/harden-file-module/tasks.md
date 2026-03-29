## 1. Integer Overflow Protection

- [ ] 1.1 Create `src/file/overflow_check.h` with inline overflow check helpers
- [ ] 1.2 Implement `check_overflow_add(uint64_t a, uint64_t b, uint64_t *result)`
- [ ] 1.3 Implement `check_overflow_sub(uint64_t a, uint64_t b, uint64_t *result)`
- [ ] 1.4 Implement `check_overflow_mul(uint64_t a, uint64_t b, uint64_t *result)`
- [x] 1.5 Update `fileReadMmap.c` to use overflow checks on view_size calculation
- [x] 1.6 Update `fileWriteMmap.c` to use overflow checks on required_size calculation
- [ ] 1.7 Update `fileAppend.c` to use overflow checks on all size calculations
- [ ] 1.8 Add overflow check tests in `tests/file_overflow/`

## 2. Resource State Tracking

- [x] 2.1 Add `fd_allocated`, `mmap_allocated` flags to `FileReadMmapState`
- [x] 2.2 Add `fd_allocated`, `mmap_allocated` flags to `FileWriteMmapState`
- [ ] 2.3 Add `cqe_consumed` flag to ring-based state structs
- [x] 2.4 Update `fileReadMmap.c` cleanup to check flags before close/unmap
- [x] 2.5 Update `fileWriteMmap.c` cleanup to check flags before close/unmap
- [ ] 2.6 Update `fileReadRing.c` cleanup to check flags
- [ ] 2.7 Update `fileWriteRing.c` cleanup to check flags
- [ ] 2.8 Add tests: force error at each allocation point, verify no double-free

## 3. io_uring CQE Consumption

- [ ] 3.1 Update `fileReadRing.c` to use `io_uring_for_each_cqe()` macro
- [ ] 3.2 Update `fileReadRing.c` to call `io_uring_cqe_seen()` after processing
- [ ] 3.3 Update `fileWriteRing.c` to use `io_uring_for_each_cqe()` macro
- [ ] 3.4 Update `fileWriteRing.c` to call `io_uring_cqe_seen()` after processing
- [ ] 3.5 Add handling for partial completions (cqe->res < expected)
- [ ] 3.6 Add handling for `IORING_CQE_F_MORE` flag
- [ ] 3.7 Track `bytes_processed` and submit remaining I/O if partial
- [ ] 3.8 Add tests: simulate partial completions, multiple CQEs, errors

## 4. Lock Timeout

- [ ] 4.1 Define `FILE_LOCK_TIMEOUT_MS` default constant (5000)
- [ ] 4.2 Define `FILE_LOCK_RETRY_INTERVAL_MS` constant (100)
- [ ] 4.3 Create `fun_file_lock_with_timeout(String filePath, uint32_t timeout_ms)`
- [ ] 4.4 Implement non-blocking lock with `LOCK_EX | LOCK_NB`
- [ ] 4.5 Implement retry loop with timeout check
- [ ] 4.6 Add monotonic time tracking for timeout
- [ ] 4.7 Define `ERROR_RESULT_LOCK_TIMEOUT` error code
- [ ] 4.8 Keep `fun_file_lock()` as wrapper with default timeout
- [ ] 4.9 Add tests: concurrent lock holders, timeout behavior, deadlock scenario

## 5. Notification Cleanup

- [x] 5.1 Add `inotify_opened`, `watch_registered` flags to `FileNotificationState`
- [x] 5.2 Update `fun_register_file_change_notification()` to set flags
- [ ] 5.3 Update `fun_unregister_file_change_notification()` to:
  - [ ] 5.3.1 Call `inotify_rm_watch()` if watch registered
  - [ ] 5.3.2 Call `close()` on inotify_fd if opened
  - [ ] 5.3.3 Free the state struct
  - [ ] 5.3.4 Clear flags after each operation
- [ ] 5.4 Add tests: unregister without register, multiple unregisters
- [ ] 5.5 Add tests: verify fd closure with `lsof` or `/proc/self/fd`

## 6. Buffer Validation

- [ ] 6.1 Update `fileReadMmap.c` to check `bytes_to_read <= output.capacity`
- [ ] 6.2 Update `fileReadRing.c` to check `bytes_to_read <= output.capacity`
- [ ] 6.3 Update `fileRead.c` to check `bytes_to_read <= output.capacity`
- [ ] 6.4 Define `ERROR_RESULT_BUFFER_TOO_SMALL` error code
- [ ] 6.5 Add tests: buffer smaller than requested, exact size, larger
- [ ] 6.6 Add tests: zero-capacity buffer, NULL buffer

## 7. Runtime Page Size

- [ ] 7.1 Remove `#define PAGE_SIZE 4096` from all files
- [ ] 7.2 Create `static inline long get_page_size()` helper with caching
- [ ] 7.3 Add `#include <unistd.h>` for `sysconf()`
- [ ] 7.4 Update `fileReadMmap.c` to use `get_page_size()`
- [ ] 7.5 Update `fileWriteMmap.c` to use `get_page_size()`
- [ ] 7.6 Add fallback to 4096 if `sysconf()` fails
- [ ] 7.7 Add tests: verify page size matches system, alignment correct

## 8. Durability Modes

- [ ] 8.1 Define `FileDurabilityMode` enum (ASYNC, SYNC, FULL)
- [ ] 8.2 Add `durability_mode` field to `FileWriteParameters`
- [ ] 8.3 Add `durability_mode` field to `FileAppendParameters`
- [ ] 8.4 Update `fileWriteMmap.c` to call `msync(MS_SYNC)` for SYNC mode
- [ ] 8.5 Update `fileWriteMmap.c` to call `fsync()` for FULL mode
- [ ] 8.6 Update `fileAppend.c` to call `fsync()` for SYNC/FULL modes
- [ ] 8.7 Add `#include <sys/mman.h>` for `msync()` constants
- [ ] 8.8 Default to `FILE_DURABILITY_ASYNC` for backward compatibility
- [ ] 8.9 Add tests: crash simulation, verify data persistence per mode

## 9. Syscall Headers

- [x] 9.1 Add `#include <sys/syscall.h>` to all files using syscalls
- [x] 9.2 Remove hardcoded `#define SYS_open`, `SYS_close`, etc.
- [x] 9.3 Verify all syscall constants available from header
- [x] 9.4 Update `fileReadMmap.c`, `fileWriteMmap.c`, `fileAppend.c`
- [x] 9.5 Update `fileLock.c`, `fileNotification.c`
- [x] 9.6 Build and test on x86_64, verify no regressions

## 10. Code Cleanup

- [x] 10.1 Remove duplicate `#include <stddef.h>` from `fileReadMmap.c`
- [x] 10.2 Remove duplicate `#include <stddef.h>` from `fileAppend.c`
- [ ] 10.3 Run `./code-format.sh` on all modified files
- [ ] 10.4 Verify no clang-format warnings

## 11. Test Coverage Expansion

- [ ] 11.1 Create `tests/file_overflow/` for integer overflow tests
- [ ] 11.2 Create `tests/file_concurrent/` for concurrent access tests
- [ ] 11.3 Create `tests/file_large/` for >2GB file tests
- [ ] 11.4 Create `tests/file_durability/` for fsync/msync tests
- [ ] 11.5 Add tests: permission denied scenarios
- [ ] 11.6 Add tests: disk full conditions
- [ ] 11.7 Add tests: interrupted syscalls (EINTR)
- [ ] 11.8 Add tests: symlink following behavior
- [ ] 11.9 Add tests: special files (devices, sockets, FIFOs)
- [ ] 11.10 Run full test suite on Linux, verify all pass

## 12. Documentation

- [ ] 12.1 Update header file comments with durability modes and guarantees
- [ ] 12.2 Document lock timeout behavior and error codes
- [ ] 12.3 Add migration guide for breaking changes
- [ ] 12.4 Update README.md with new robustness features
- [ ] 12.5 Add example code for each durability mode

## 13. Validation

- [ ] 13.1 Run `openspec validate harden-file-module`
- [ ] 13.2 Run `openspec validate --specs` for this change
- [x] 13.3 Run full test suite: `./run-tests-linux-amd64.sh`
- [ ] 13.4 Run code format: `./code-format.sh`
- [ ] 13.5 Performance benchmark: verify <1% overhead
- [ ] 13.6 Fuzz test: run with AFL or libFuzzer for 24 hours
- [ ] 13.7 Code review: security-focused review of all changes
