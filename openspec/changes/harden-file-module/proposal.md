# Harden File Module

## Why

The file module has critical robustness and correctness issues that could lead to crashes, data corruption, or undefined behavior:

1. **Integer overflow** in size calculations can cause buffer overflows
2. **Resource leak paths** when errors occur mid-operation
3. **io_uring CQE handling** doesn't properly consume completions
4. **File locking** can deadlock with no timeout
5. **File notification** leaks fds and memory on unregister
6. **No buffer validation** before memory copies
7. **Hardcoded page size** breaks on non-x86 architectures
8. **No durability guarantees** - data loss on crash
9. **Hardcoded syscall numbers** break portability

## What

Add robustness, correctness, and portability fixes without changing the public API (except where noted in breaking changes).

## Capabilities

| Capability | Description |
|------------|-------------|
| `integer-overflow-checks` | Overflow validation on all arithmetic |
| `resource-state-tracking` | Explicit flags prevent double-free/close |
| `io_uring-cqe-consumption` | Proper CQE handling with `io_uring_cqe_seen()` |
| `lock-timeout` | Configurable timeout with retry mechanism |
| `notification-cleanup` | Proper fd close and state free on unregister |
| `buffer-validation` | Capacity check before memory copies |
| `runtime-page-size` | `sysconf(_SC_PAGESIZE)` instead of hardcoded 4096 |
| `durability-modes` | ASYNC/SYNC/FULL modes with `msync()`/`fsync()` |
| `syscall-headers` | `<sys/syscall.h>` instead of hardcoded numbers |

## Impact

- **Breaking:** Lock operations now accept timeout parameter
- **Breaking:** Write operations accept durability mode parameter
- **Non-breaking:** All other changes are internal hardening

## Excluded

The following issues are explicitly out of scope for this change:
- Path sanitization / traversal prevention
- TOCTOU race conditions
- File permission validation

These will be addressed in a separate security-focused change.
