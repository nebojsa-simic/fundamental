## Context

The Fundamental Library avoids the C standard library. All public APIs use `fun_*` prefixes and the library provides replacements for common stdlib functions: `fun_console_*` for output, `fun_memory_*` for allocation and memory operations, `fun_string_*` for string handling, `fun_math_*` for math, and `fun_*_file_*` for file I/O. The test suite (~35 files across ~25 directories) still uses stdlib extensively: ~200 `printf` calls, ~80 `malloc`/`free` calls, ~300 `assert()` macros, and ~50 `FILE*` operations. This is a code hygiene issue — tests should demonstrate the library they test.

## Goals / Non-Goals

**Goals:**
- Remove all stdlib includes and calls from test `.c` files
- Replace with equivalent `fun_*` library calls
- Pre-create file fixtures for file-related tests and check them in
- Keep test logic unchanged — this is a mechanical substitution, not a rewrite

**Non-Goals:**
- `tests/math/tools/generate_golden.c` — deferred (needs float precision formatting not yet in `fun_string_template`)
- Library API changes — no new library functions are being added
- Test restructuring — test function names, flow, and assertions remain the same
- Linux-only test files (`test_linux.c`) — handled alongside their Windows counterparts

## Decisions

### Decision 1: Pre-created fixtures over build-script generation

**Chosen**: Check empty and small content fixture files into the repository.

**Alternatives considered**:
- Build script creates fixtures: adds complexity, platform-specific shell commands, race conditions
- `fun_write_file_in_memory` in setup: tests testing file I/O using file I/O — circular

**Rationale**: Fixture files are small (mostly empty `.txt` files, a few with known content like "Initial "). They change rarely and are data, not code. Checking them in is the simplest option.

### Decision 2: `check_int` pattern over `assert.h` macros

**Chosen**: Use `check_int(condition, &tc, "message")` as seen in `tests/math/test_edge_cases.c`.

**Alternatives considered**:
- Keep `assert()` but redefine: `assert` is a macro from `<assert.h>`, can't work without the header
- `fun_error_*` return types: overkill for test assertions (tests are not library code, don't need to return errors)

**Rationale**: The `check_int` pattern already exists in the math edge case tests. It provides named failure messages, counts passes/fails, and doesn't abort the process on first failure. It's easy to replicate.

### Decision 3: `fun_read_file_in_memory` for verification, not `fun_stream_*`

**Chosen**: Use `fun_read_file_in_memory` to read back written files for verification.

**Rationale**: File tests currently do `fopen → fread → memcmp → fclose`. These are small verification reads (256 bytes max). `fun_read_file_in_memory` is the simplest equivalent — one call that returns the whole file in a buffer. The stream API is overkill for reading a 12-byte test fixture.

### Decision 4: Phased execution

**Chosen**: Three phases — mechanical substitution, assert pattern change, FILE I/O cleanup.

**Rationale**: Phase 1 (printf, strcmp, strlen, memcpy, memset, snprintf) is purely mechanical and low-risk. Phase 2 (assert, malloc/free) changes the test assertion pattern but only in test files. Phase 3 (FILE I/O) requires pre-creating fixtures. Grouping by risk lets each phase be verified independently before proceeding.

### Decision 5: `fun_string_template` for path formatting and config strings

**Chosen**: Replace `snprintf(buf, sz, "%s/%s_golden.h", dir, name)` with `fun_string_template("${dir}/${name}_golden.h", params, ...)`.

**Rationale**: The template API covers `${key}` (string), `#{key}` (int), and `%{key}` (double). Path building and `KEY=VALUE` config strings use only string and int placeholders. Float precision formatting (`.9g`, `.1f`) is the one gap — that's why `generate_golden.c` is deferred.

## Risks / Trade-offs

- **Risk**: Pre-created fixtures might not exist on clean checkout. Mitigation: fixtures are checked into git alongside test source.
- **Risk**: `check_int` implementation varies across test files, creating inconsistency. Mitigation: follow the exact pattern from `test_edge_cases.c`, use `TestCount` struct consistently.
- **Risk**: Filesystem test fixtures in `test_output/` may conflict with existing test artifacts. Mitigation: fixtures go in the test directory root as flat files (e.g., `test_lock_basic.txt`), not in a shared `test_output/` directory.
- **Trade-off**: Test files grow slightly (template calls more verbose than `snprintf`). Accepted — clarity and library dogfooding outweigh brevity.
