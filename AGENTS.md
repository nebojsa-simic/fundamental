# AGENTS.md - Fundamental Library Development Guide

## 🤖 AI Agents: Use the fundamental-expert Skill

**Before writing ANY demo or test code:**

```
/fundamental-expert
```

This skill provides:
- ✅ **Correct API patterns** - Knows exact function names and signatures
- ✅ **Build script templates** - Has all source file locations
- ✅ **Common pitfalls** - Warns about mistakes before you make them
- ✅ **Working examples** - Copy-paste patterns for every module

**The skill was created from the codebase and knows:**
- All module interfaces and implementations
- Platform-specific source locations (`arch/*/`)
- Correct async/streaming patterns
- Memory management requirements

**Using the skill prevents:**
- ❌ Wrong function names (e.g., `fun_file_read()` vs `fun_read_file_in_memory()`)
- ❌ Missing source files in build scripts
- ❌ Stdlib dependencies in library code
- ❌ Memory leaks from un-freed allocations

---

## ⚠️ CRITICAL: Before Writing Demo/Test Code

**MUST complete ALL steps before writing code:**

1. **Load the skill** - `/fundamental-expert` and ask for patterns
2. **Read the header** - `cat include/fundamental/<module>/<module>.h` - find exact function names
3. **Copy test build patterns** - `cat tests/<module>/build-windows-amd64.bat` - exact source files
4. **Verify source locations** - `ls src/<module>/` AND `ls arch/<module>/<platform>/`
5. **String is typedef** - `typedef const char *String` - just pass `"text"` directly
6. **File I/O is async** - `fun_read_file_in_memory(Read)` NOT `fun_file_read()`
7. **File sources in arch/** - `arch/file/<platform>/fileRead*.c` NOT `src/file/`
8. **Write minimal first** - Compile before adding features
9. **Build immediately** - No features until clean compile
10. **Test both platforms** - Windows AND Linux
11. **Run validator** - `../../demos/validate-demo.bat` before committing

**Skipping these steps guarantees wasted time.**

## Build Commands

### Full Test Suite
- **Windows**: `run-tests-windows-amd64.bat` - Run all tests by iterating through `tests\*` directories
- **Linux**: `./run-tests-linux-amd64.sh` - Run all tests by iterating through `tests/*/` directories

### Individual Component Builds
- **Windows**: Navigate to specific test directory and run: `.\build-windows-amd64.bat`
- **Linux**: Navigate to specific test directory and run: `./build-linux-amd64.sh`
- Individual test executables are named `test.exe` (Windows) or `test` (Linux)

### Running Individual Tests
- Each test directory contains its own build script
- Execute test after successful build: `.\test.exe` (Windows) or `./test` (Linux)

## Lint Commands
- **Windows**: `code-format.bat` - Runs `clang-format -i -style=file` on all `.c` and `.h` files
- **Linux**: `./code-format.sh` - Unix equivalent using bash script
- Code formatter uses `.clang-format` configuration (based on Linux kernel style)

## Test Commands
- Individual tests: Run specific `test.exe` (Windows) or `test` (Linux) in each component's test directory
- Batch testing (Windows): Execute `run-tests-windows-amd64.bat` to run all test suites sequentially
- Batch testing (Linux): Execute `./run-tests-linux-amd64.sh` to run all test suites sequentially
- Each test follows the same pattern:
  - Compile with build script  
  - Execute if compilation succeeds
  - Return exit code for failure

### Test Modules
Current test directories:
- `async/` - Async operations and process spawn
- `collections/` - Dynamic arrays
- `console/` - Console I/O
- `filesystem/` - Directory and path operations
- `hashmap/` - Hash map operations
- `memory/` - Memory allocation
- `process_spawn/` - Process execution
- `rbtree/` - Red-black trees
- `set/` - Set operations
- `stream/` - Stream I/O
- `string*/` - String operations (conversion, operations, template)
- `file*/` - File I/O (read, write, append, lock)

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

### OpenSpec CLI Priority Rule

**CRITICAL**: Before manually implementing any OpenSpec workflow steps:

1. **Check for built-in CLI commands first**:
   ```bash
   openspec --help                    # Discover all available commands
   openspec <command> --help          # Get command-specific options
   ```

2. **Prefer CLI commands over manual implementation**:
   - `openspec archive <change>` - Archives changes and syncs specs automatically (DO NOT implement manually)
   - `openspec status --change <name>` - Check artifact completion status
   - `openspec instructions apply --change <name>` - Get implementation guidance
   - `openspec validate <change>` - Validate changes and specs

3. **Manual implementation is ONLY needed when**:
   - No CLI command exists for the operation
   - The CLI explicitly delegates to manual steps

**Why this matters**: Skills describe the *workflow*, but the CLI may automate it. Always discover commands before implementing manually.

### OpenSpec CLI Error Handling

**When the CLI reports warnings or errors:**

1. **DO NOT bypass the CLI** - Never manually move directories, edit OpenSpec files, or run git commands to "fix" what the CLI is handling
2. **Read the error carefully** - The CLI will tell you exactly what needs fixing (e.g., missing sections, incomplete tasks)
3. **Fix the underlying issue** - Update the spec/tasks/proposal as the error indicates
4. **Re-run the CLI command** - Let the CLI complete the operation after fixes are applied

**Explicitly Forbidden:**
- ❌ `mv openspec/changes/<name> openspec/changes/archive/` - Use `openspec archive <name>` instead
- ❌ `git add openspec/...` for OpenSpec changes - The CLI handles commits
- ❌ `git commit -m "Archive..."` for OpenSpec archives - The CLI handles commits
- ❌ Manually editing tasks.md to mark tasks complete just to satisfy archive - Fix actual implementation

**If the CLI fails repeatedly:**
1. Document the exact error message
2. Ask the user for guidance before attempting workarounds
3. The CLI is the source of truth - if it says something is wrong, it is wrong

### OpenSpec Archive Checklist

Before archiving a change, verify:

- [ ] All tasks in `tasks.md` are marked complete (`- [x]`)
- [ ] All artifacts show status `done` in `openspec status --change <name>`
- [ ] Delta specs are synced (run sync if prompted)
- [ ] **Run `openspec archive <name>` and let it handle everything**

**After running `openspec archive`:**
- The CLI will move the directory to `openspec/changes/archive/`
- The CLI will sync specs to `openspec/specs/`
- The CLI will commit the changes
- **DO NOT** run `git add` or `git commit` for OpenSpec archive operations

### OpenSpec Archive: Never Do This

**The following actions are explicitly forbidden when archiving OpenSpec changes:**

| ❌ Forbidden | ✅ Correct Approach |
|-------------|-------------------|
| `mv openspec/changes/<name> openspec/changes/archive/` | `openspec archive <name>` |
| `git add openspec/changes/archive/` | CLI handles this automatically |
| `git commit -m "Archive..."` | CLI handles this automatically |
| Manually editing tasks.md checkboxes | Fix actual implementation, then run CLI |
| Bypassing CLI validation errors | Fix the reported issues, then retry CLI |
| Running archive twice to "force" it | Ask user for guidance if CLI fails |

**If `openspec archive` reports errors:**
1. Read the error message - it tells you exactly what's wrong
2. Fix the underlying issue (incomplete tasks, missing spec sections, etc.)
3. Re-run `openspec archive <name>`
4. If it still fails, ask the user - do NOT attempt manual workarounds

**Remember:** The CLI is the source of truth. If validation fails, something is genuinely incomplete.

---

## AI Agent Skills

This project includes specialized skills for AI coding agents (Opencode, Claude Code) that provide copy-paste examples for common Fundamental Library operations.

### Available Skills

Skills are located in `.opencode/skills/` and cover these domains:

| Skill | File | Description |
|-------|------|-------------|
| **File I/O** | `fundamental-file-io.md` | Read, write, append files, stream-based I/O |
| **Memory** | `fundamental-memory.md` | Allocate, free, copy, fill, compare memory |
| **Console** | `fundamental-console.md` | Output text, progress bars, error messages |
| **Directory** | `fundamental-directory.md` | Create, list, remove directories, iterate files |
| **String** | `fundamental-string.md` | Copy, join, template, convert, compare strings |
| **Collections** | `fundamental-collections.md` | Arrays, hashmaps, sets, red-black trees |
| **Async** | `fundamental-async.md` | Await results, poll status, spawn processes |
| **Config** | `fundamental-config.md` | Load configuration, cascade sources, get values |
| **Index** | `fundamental-skills-index.md` | Central index with cross-references |

### Using Skills

**For AI Agents:** When implementing Fundamental Library code:

1. **Identify the task**: "I need to read a file"
2. **Find the skill**: See table above or check `fundamental-skills-index.md`
3. **Copy the pattern**: Use the example as a template
4. **Adapt to context**: Modify paths, sizes, error handling as needed

**Example Workflow:**
```
User: "Read a config file and parse it"

Agent workflow:
1. Load fundamental-file-io.md for file reading pattern
2. Load fundamental-memory.md for buffer allocation
3. Load fundamental-string.md for string parsing or templating
4. Combine patterns into working code
```

### Skill Format

All skills follow this structure:
- **Quick Reference** - Table of common tasks and functions
- **Task Examples** - Copy-paste code with full error handling
- **Key Points** - Important notes and gotchas
- **See Also** - Links to related skills and headers

### Design Principles

Skills reinforce Fundamental Library patterns:
- **Allocate → Operate → Check Error → Use → Cleanup** - Consistent flow in all examples
- **Error handling mandatory** - Every example shows error checking
- **Memory safety** - Every allocation has corresponding free
- **Cross-references** - Skills link to related skills for discovery

### For Users

Skills can serve as:
- **Usage examples** - See how to use library functions correctly
- **Learning resource** - Understand library patterns and conventions
- **Quick reference** - Look up common operations without reading headers

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
- Each directory should have platform-specific build scripts:
  - Windows: `build-windows-amd64.bat`
  - Linux: `build-linux-amd64.sh`
- Tests should include validation for success and error conditions
- Test names start with `test_` and follow function-under-test naming
- Include negative tests (expected failures) to verify error handling