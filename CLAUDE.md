# Fundamental Library - Claude Code Instructions

## Overview

Zero-stdlib C library for cross-platform CLI applications. Complete reimplementation of C standard library functionality with no stdlib dependencies.

## Build Commands

### Full Test Suite
- **Windows**: `run-tests-windows-amd64.bat`
- **Linux**: `./run-tests-linux-amd64.sh`

### Individual Component
- Navigate to `tests/<component>/` and run `build-windows-amd64.bat` (or `build-linux-amd64.sh`)
- Test executables: `test.exe` (Windows) or `test` (Linux)

### Code Formatting
- **Windows**: `code-format.bat` (runs `clang-format -i -style=file` on all `.c` and `.h` files)
- **Linux**: `./code-format.sh`
- Config: `.clang-format` (Linux kernel style, 4-space indent, 80-col limit)

## Adding a New Module — Checklist

Every new module requires all of the following. Do not consider a module done until every item is complete.

**In this repo (`fundamental/`):**
- [ ] `include/<module>/<module>.h` — public API header (types, `DEFINE_RESULT_TYPE`, function declarations)
- [ ] `src/<module>/<module>.c` — core implementation, no `#ifdef`, calls arch-layer functions for anything platform-specific
- [ ] `arch/<module>/windows-amd64/<module>.c` — Windows AMD64 arch implementation
- [ ] `arch/<module>/linux-amd64/<module>.c` — Linux AMD64 arch implementation
- [ ] `tests/<module>/test_<module>.c` — unit tests covering success and error paths
- [ ] `tests/<module>/build-windows-amd64.bat` — Windows test build script
- [ ] `tests/<module>/build-linux-amd64.sh` — Linux test build script (`git update-index --chmod=+x` to set execute bit)
- [ ] `openspec/specs/<module>/spec.md` — OpenSpec specification
- [ ] `.opencode/skills/fundamental-<module>.md` — agent skill file with Quick Reference and copy-paste examples
- [ ] `.opencode/skills/fundamental-skills-index.md` — add entry to skills index and cross-reference map
- [ ] `CLAUDE.md` — add row to Modules table and Skills table

**In `fundamental-cli/` (after vendoring):**
- [ ] Run `vendor-fundamental.bat` to copy latest sources into `vendor/fundamental/` (xcopy does not delete removed files — manually remove any stale files from `vendor/fundamental/` after vendoring)
- [ ] `build-windows-amd64.bat` — add `src/<module>/<module>.c` and `arch/<module>/windows-amd64/<module>.c`
- [ ] `build-linux-amd64.sh` — add `src/<module>/<module>.c` and `arch/<module>/linux-amd64/<module>.c`

## Architecture

```
arch/       Platform-specific code (linux-amd64, windows-amd64)
include/    Public API headers
src/        Core implementations
tests/      Unit tests (one directory per module)
openspec/   Specifications and change management
```

Platform-specific code goes ONLY in `arch/`. Never put OS-specific logic in `src/` or `include/`.

## Design Principles (NEVER violate)

1. **No C stdlib** - No `#include <stdio.h>`, no `malloc()`, no `printf()`. Use `fun_` functions.
2. **Caller-allocated memory** - Functions don't allocate for the caller. Pass pre-allocated buffers.
3. **Explicit error handling** - All fallible functions return `Result` types. Always check with `fun_error_is_error()`.
4. **Descriptive naming** - `fun_` prefix, full names: `fun_string_from_int()` not `fun_itoa()`.
5. **Cross-platform** - No OS logic outside `arch/`. Use file/async modules, not direct syscalls.

## Code Style

- Include: `#include "..."` (no angle brackets except system headers), local first
- Header guards: `#ifndef LIBRARY_*_H`
- Types uppercase: `Memory`, `String`, `FileStream`
- Functions: `fun_*` prefix, fully descriptive
- Error types: `CanReturnError(ActualType)` macro, `DEFINE_RESULT_TYPE(TypeName)`
- Constants: `ERROR_CODE_*` uppercase with underscores
- Formatting: K&R braces, pointer alignment `Type *variable`
- Memory: `fun_memory_allocate()` / `fun_memory_free()`, caller always frees

## Modules

| Module | Status | Key Functions |
|--------|--------|---------------|
| Memory | Complete | `fun_memory_allocate()`, `fun_memory_free()` |
| String | Complete | `fun_string_copy()`, `fun_string_template()`, conversions |
| Error | Complete | `DEFINE_RESULT_TYPE()`, `fun_error_is_error()` |
| Async | Complete | `fun_async_await()`, process spawn |
| Console | Complete | `fun_console_write()`, `fun_console_write_line()` |
| File | Complete | `fun_read_file_in_memory()`, `fun_write_memory_to_file()` |
| Stream | Complete | `fun_stream_create_file_read()`, `fun_stream_read()` |
| Filesystem | Complete | `fun_filesystem_create_directory()`, path utils |
| Platform | Complete | `fun_platform_get()`, OS/arch detection via arch layer |
| Collections | Complete | Arrays, HashMaps, RB-Trees, Sets |
| Config | In Dev | `fun_config_load()`, cascading CLI > env > INI |

## Test Organization

Each test directory under `tests/` has:
- Platform build scripts (`build-windows-amd64.bat`, `build-linux-amd64.sh`)
- Test binary (`test.exe` / `test`)
- Tests validate both success and error conditions
- Test names follow `test_<function_under_test>` pattern

Test modules: async, collections, console, filesystem, hashmap, memory, process_spawn, rbtree, set, stream, string_*, file_*

## OpenSpec Workflow

This project uses OpenSpec for change management:
- `openspec/specs/` - Capability specifications (Gherkin)
- `openspec/changes/` - Active changes with artifacts (proposal, design, tasks, specs)
- Commands: `/opsx-explore`, `/opsx-propose`, `/opsx-apply`, `/opsx-archive`
- CLI: `openspec list`, `openspec status`, `openspec validate`, `openspec new change`

## AI Agent Skills

Specialized skills in `.opencode/skills/` provide copy-paste examples for common operations:

| Skill | File | Domain |
|-------|------|--------|
| File I/O | `fundamental-file-io.md` | Read, write, append, stream I/O |
| Memory | `fundamental-memory.md` | Allocate, free, copy, fill, compare |
| Console | `fundamental-console.md` | Output, progress bars, errors |
| Directory | `fundamental-directory.md` | Create, list, remove, iterate |
| String | `fundamental-string.md` | Copy, join, template, convert |
| Collections | `fundamental-collections.md` | Arrays, hashmaps, sets, RB-trees |
| Async | `fundamental-async.md` | Await, poll, spawn processes |
| Config | `fundamental-config.md` | Load config, cascade sources |
| Platform | `fundamental-platform.md` | Detect OS/arch, convert to string |
| Index | `fundamental-skills-index.md` | Central cross-reference |

**Usage**: Identify task, find matching skill, copy the pattern, adapt to context. All examples follow the `Allocate -> Operate -> Check Error -> Use -> Cleanup` flow.

## Common Patterns

### Error Handling
```c
MemoryResult result = fun_memory_allocate(1024);
if (fun_error_is_error(result.error)) {
    return 1;
}
Memory buffer = result.value;
// ... use buffer ...
voidResult free_result = fun_memory_free(&buffer);
```

### Async I/O
```c
AsyncResult result = fun_read_file_in_memory(params);
fun_async_await(&result);
if (result.status == ASYNC_COMPLETED) { /* success */ }
```

### String Templates
```c
// Prefixes: ${string} #{int} %{double} *{pointer}
StringTemplateParam params[] = {
    { "name", { .stringValue = "Alice" } },
    { "count", { .intValue = 42 } }
};
fun_string_template("Hello ${name}, #{count} items", params, 2, output);
```
