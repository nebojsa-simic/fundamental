## ADDED Requirements

### Requirement: Test files use library primitives only
Test source files SHALL NOT include or call functions from `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<assert.h>`, or `<math.h>`. Test output SHALL use `fun_console_write` / `fun_console_write_line`, memory operations SHALL use `fun_memory_allocate` / `fun_memory_free` / `fun_memory_copy` / `fun_memory_fill`, string operations SHALL use `fun_string_compare` / `fun_string_length` / `fun_string_template`, validation SHALL use the `check_int` pattern, file I/O verification SHALL use `fun_read_file_in_memory`, and floating-point reference math SHALL use `fun_math_*` functions.

#### Scenario: No stdio includes remain
- **WHEN** any test `.c` file in `tests/*/` is inspected (excluding `tests/math/tools/generate_golden.c`)
- **THEN** it includes only `"fundamental/..."` headers and `<stdint.h>` / `<stddef.h>` / `<stdbool.h>` for type definitions

#### Scenario: Console output uses library
- **WHEN** any test prints a result or status message
- **THEN** it calls `fun_console_write` or `fun_console_write_line`

#### Scenario: Memory operations use library
- **WHEN** any test allocates, frees, copies, or fills memory
- **THEN** it calls `fun_memory_allocate`, `fun_memory_free`, `fun_memory_copy`, or `fun_memory_fill`

#### Scenario: String comparison uses library
- **WHEN** any test compares strings
- **THEN** it calls `fun_string_compare` instead of `strcmp`

#### Scenario: File I/O verification uses library read
- **WHEN** a file-related test verifies written content
- **THEN** it calls `fun_read_file_in_memory` instead of `fopen` / `fread` / `fclose`

#### Scenario: File fixtures are pre-created
- **WHEN** a file-related test needs an existing file on disk
- **THEN** the file is checked into the repository as a fixture rather than created with `fopen` / `fwrite` at test time

#### Scenario: Validation uses check_int pattern
- **WHEN** a test validates a condition
- **THEN** it uses a `check_int` wrapper or error-check macro rather than the `assert()` macro from `<assert.h>`
