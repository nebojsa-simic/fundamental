## 1. Fixture Files

- [x] 1.1 Create empty fixture files for file_concurrent tests
- [x] 1.2 Create content fixture files for fileAppend and file_exists
- [x] 1.3 Create empty fixture files for fileLock tests
- [x] 1.4 Create fixture files for remaining file tests

## 2. Phase 1: Mechanical Substitution

- [x] 2.1 Replace `printf` / `fprintf` with `fun_console_write` / `fun_console_write_line` across all test files
- [x] 2.2 Replace `strcmp` with `fun_string_compare`
- [x] 2.3 Replace `strlen` with `fun_string_length`
- [x] 2.4 Replace `memcpy` with `fun_memory_copy`
- [x] 2.5 Replace `memset` with `fun_memory_fill`
- [x] 2.6 Replace `snprintf` with `fun_string_template`
- [x] 2.7 Remove `#include <stdio.h>` and `#include <string.h>` from all test files where all calls are replaced
- [x] 2.8 Build and run all affected tests on Windows; verify zero regressions

## 3. Phase 2: Assert Pattern Change

- [x] 3.1 Replace `assert()` calls with inline `if` checks in collections, async, fileWrite, fileLock, fileRead, file_large, hashmap, memory, string*, console, object-pool, set, rbtree, sync, network*, thread-pool
- [x] 3.2 Remove `#include <assert.h>` from all test files
- [x] 3.3 Build and run all affected tests on Windows; verify zero regressions

## 4. Phase 3: File I/O Cleanup

- [x] 4.1 Replace `fopen` / `fread` / `fclose` verification blocks with `fun_read_file_in_memory`
- [x] 4.2 Remove `fopen` / `fclose` fixture-creation calls in file_concurrent
- [x] 4.3 Remove `fopen` / `fprintf` / `fclose` / `fwrite` fixture-creation calls in fileLock, file_exists, fileAppend
- [x] 4.4 Remove `remove()` cleanup calls
- [x] 4.5 Remove `<stdio.h>` from all file test files
- [x] 4.6 Build and run all affected tests on Windows; verify zero regressions

## 5. Phase 4: Math Test Memory

- [x] 5.1 Replace `malloc` / `free` with `fun_memory_allocate` / `fun_memory_free` in test_consistency.c
- [x] 5.2 Replace `malloc` / `free` with `fun_memory_allocate` / `fun_memory_free` in test_edge_cases.c
- [x] 5.3 Replace `malloc` / `free` with `fun_memory_allocate` / `fun_memory_free` in test_performance.c
- [x] 5.4 Replace `malloc` / `free` with `fun_memory_allocate` / `fun_memory_free` in test_vector_accuracy.c
- [x] 5.5 Remove `<stdlib.h>` from all test files
- [x] 5.6 Build and run `tests/math` on Windows; verify all 696k+ tests pass (696145/696145)

## 6. Cross-Platform Verification

- [x] 6.1 Build and run all modified tests on Windows; verify green
- [x] 6.2 Build and run `tests/math` on Linux via WSL; verify all 696k+ tests pass
- [x] 6.3 Run `code-format.bat`
- [x] 6.4 `openspec validate --changes` passes
