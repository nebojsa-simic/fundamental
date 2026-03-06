# AGENTS.md - Fundamental Library Development Guide

## Build Commands

### Full Test Suite
- `run-tests-windows-amd64.bat` - Run all tests by iterating through `tests\*` directories

### Individual Component Builds
- Navigate to specific test directory and run: `.\build-windows-amd64.bat`
- Individual test executables are named `test.exe`

### Running Individual Tests
- Each test directory contains its own build script: `build-windows-amd64.bat`
- Execute `test.exe` after successful build: `.\test.exe`

## Lint Commands
- `code-format.bat` - Runs `clang-format -i -style=file` on all `.c` and `.h` files in directory tree
- `code-format.sh` - Unix equivalent using bash script
- Code formatter uses `.clang-format` configuration (based on Linux kernel style)

## Test Commands
- Individual tests: Run specific `test.exe` in each component's test directory
- Batch testing: Execute `run-tests-windows-amd64.bat` to run all test suites sequentially
- Each test follows the same pattern:
  - Compile with build script  
  - Execute if compilation succeeds
  - Return exit code for failure

## OpenSpec Workflow
This project uses OpenSpec for specifications and change management:
- Specifications stored as Markdown in `openspec/specs/`
- Changes managed in `openspec/changes/` with corresponding artifacts:
  - `proposal.md` - Change proposals and justification
  - `design.md` - Technical design and architecture decisions  
  - `spec.md` - Detailed functional specifications in Gherkin format
  - `tasks.md` - Engineering tasks and implementation breakdown
- Validate changes with: `openspec validate <change-name>`
- Run validation across all specs: `openspec validate --specs`
- Run validation across all changes: `openspec validate --changes`

### OpenSpec Skill-Based Commands:
AI agents can use specialized OpenSpec skills for guided workflow assistance:
- `openspec-propose` - Propose a new change with all artifacts generated in one step. Use when describing what to build and get a complete proposal with design, specs, and implementation tasks.
- `openspec-explore` - Enter exploration mode - a thinking partner for investigating problems, clarifying requirements, and exploring solutions before implementation.
- `openspec-apply-change` - Implement tasks from an OpenSpec change. Use when implementing, continuing, or working through implementation tasks.
- `openspec-archive-change` - Archive a completed change in the experimental workflow. Use when finalizing and archiving changes after implementation completion.

### OpenSpec Command Examples:
- List all available changes: `openspec list` 
- Validate specific module: `openspec validate memory` or `openspec validate async`
- Validate entire architecture: `openspec validate --all`
- Check change status: `openspec validate <change-id>`

## Code Style Guidelines

### Import Standards
- Include headers as: `#include "..."` (no angle brackets except for system headers)
- Order: local includes first, then external dependencies
- Headers should use guards: `#ifndef LIBRARY_*_H ...`

### Formatting Patterns
- Code formatting: Use `clang-format` with configuration from this repo's `.clang-format`
- Indentation: 4 spaces (soft tabs), tab width 4
- Column limit: 80 characters
- Pointer alignment: `Type *variable` (Right alignment)
- Braces: K&R style with custom formatting

### Type Conventions
- Types start with uppercase: `Memory`, `String`, `FileStream`
- Function parameters use: `typedef` for custom types, `typedef struct` for compound types
- Error types follow `ErrorResult` pattern with `code` and `message` fields
- Return types use `CanReturnError(ActualType)` macro syntax
- Define result types with: `DEFINE_RESULT_TYPE(TypeName)` macro

### Naming Conventions
- All functions start with prefix: `fun_*`
- Functions are fully descriptive: `fun_stream_create_file_read` not abbreviated
- Variables are descriptive and readable
- Constants are uppercase with underscores: `ERROR_VALUE`

### Error Handling Patterns
- All error-prone functions return types from `CanReturnError()` macro family
- Check errors with: `fun_error_is_error(result.error)` and `fun_error_is_ok(result.error)`
- Explicit error condition checks in all functions that can fail
- Error codes are positive integers with `ERROR_CODE_*` constants
- Memory failures return with specific memory error codes

### Memory Management 
- Caller-allocated memory pattern: functions don't allocate internal memory unless for temporary use
- Allocation managed with: `fun_memory_allocate()` and related functions
- All allocated memory must be freed with: `fun_memory_free()`
- No dynamic allocation in user-facing structures except by design

### Architecture Principles
- Cross-platform compatibility: No OS-specific logic in library code (only in `/arch` directory)
- Functions must work identically on all platforms
- Platform abstractions go through architecture-specific implementations in `arch/*/` directories  
- All public interfaces located in `include/*/` directories
- Implementation code in `src/*/` directories

### Async Pattern
- Async operations follow `AsyncResult`/`AsyncStatus` pattern for non-blocking operations
- Use `fun_async_await(&result)` for blocking wait
- Status values: `ASYNC_PENDING`, `ASYNC_COMPLETED`, `ASYNC_ERROR`
- Functions return immediately as `AsyncResult` which can be polled later

### Test Organization
- Test directories follow: `tests/component_name/` structure
- Each directory should have `build-windows-amd64.bat` script to compile tests
- Tests should include validation for success and error conditions
- Test names start with `test_` and follow function-under-test naming
- Include negative tests (expected failures) to verify error handling