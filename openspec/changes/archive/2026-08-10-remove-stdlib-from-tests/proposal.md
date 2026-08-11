## Why

The Fundamental Library explicitly avoids stdlib — every test should demonstrate that commitment. Currently ~35 test files across ~25 test directories use `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<math.h>`, and `<assert.h>` for output, memory, string operations, math, and file I/O. These are the exact functions the library replaces with `fun_console_*`, `fun_memory_*`, `fun_string_*`, `fun_math_*`, and `fun_*_file_*`. Cleaning this up makes the tests self-documenting examples of library usage and catches any gaps where the library's own primitives don't cover real-world test patterns.

## What Changes

- Replace `printf` / `fprintf` with `fun_console_write` / `fun_console_write_line` across all test files
- Replace `strcmp` with `fun_string_compare`, `strlen` with `fun_string_length`
- Replace `memcpy` with `fun_memory_copy`, `memset` with `fun_memory_fill`
- Replace `malloc` / `free` with `fun_memory_allocate` / `fun_memory_free` in math tests
- Replace `snprintf` (path/format patterns) with `fun_string_template`
- Replace `assert()` macros with `check_int` / error-check patterns consistent with `tests/math/test_edge_cases.c`
- Replace `FILE*` / `fopen` / `fread` / `fwrite` / `fclose` / `fprintf` / `remove` with pre-created fixture files and `fun_read_file_in_memory` for verification
- Remove `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<assert.h>` includes from all test files
- Add fixture files (empty `.txt` files and small content files) for file-related tests, checked into the repository
- Defer `tests/math/tools/generate_golden.c` — it needs precision float formatting (`.9g`, `.1f`) not covered by `fun_string_template`

## Capabilities

### New Capabilities
<!-- No new library capabilities — this is a test-side cleanup only. -->

### Modified Capabilities
<!-- No spec-level behavior changes — tests are not part of the library API. -->

## Impact

- ~35 test `.c` files modified across `tests/math/`, `tests/collections/`, `tests/config/`, `tests/async/`, `tests/fileWrite/`, `tests/fileLock/`, `tests/file_durability/`, `tests/file_concurrent/`, `tests/file_notification/`, `tests/file_overflow/`, `tests/fileAppend/`, `tests/fileRead/`, `tests/file_exists/`, `tests/file_large/`, `tests/stream/`, `tests/memory/`, `tests/string*`, `tests/console/`, `tests/tsv/`, `tests/json/`, `tests/object-pool/`, `tests/hashmap/`, `tests/set/`, `tests/rbtree/`, `tests/shutdown/`, `tests/sync/`, `tests/network*/`, `tests/thread-pool/`, `tests/platform/`, `tests/path_type/`, `tests/filesystem/`
- New fixture files checked into `tests/*/` directories for file-related tests
- ~500 function call sites replaced
- No changes to library source or headers
- `generate_golden.c` excluded (deferred)
