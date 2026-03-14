## Context

The Fundamental Library provides filesystem operations including directory creation, removal, and listing, but lacks basic file/directory existence checking functions. Currently, users must attempt operations and handle errors to determine if a path exists.

The library follows a cross-platform architecture with:
- Public API in `include/filesystem/filesystem.h`
- Platform-specific implementations in `arch/filesystem/<platform>/`
- Existing error codes for directory states (EXISTS, NOT_FOUND, etc.)

Current platform implementations already use `stat()` (POSIX) and `GetFileAttributes()` (Windows) internally for directory operations.

## Goals / Non-Goals

**Goals:**
- Add three existence-checking functions: `fun_file_exists()`, `fun_directory_exists()`, `fun_path_exists()`
- Return boolean values wrapped in typed result structures for clear semantics
- Support both Windows and POSIX platforms with native APIs
- Follow existing error handling patterns in the library
- Minimal code changes - leverage existing architecture patterns

**Non-Goals:**
- No async variants - existence checks are fast, blocking operations
- No file type detection beyond file vs directory distinction
- No permission checking beyond basic accessibility
- No symbolic link resolution or special file handling

## Decisions

### Decision 1: Return Type Design
**Choice:** Return `boolResult` (already defined in error.h) instead of adding new error codes

**Rationale:**
- Existence checks are yes/no questions - boolean is semantically clear
- `boolResult` already exists in the error module, no new types needed
- Error codes in the result handle exceptional cases (invalid path, permission denied)
- Simpler API: `if (result.value)` vs checking specific error codes

**Alternatives considered:**
- Return error codes only (FILE_EXISTS, FILE_NOT_EXISTS): More verbose, less intuitive
- Return enum with states: Overkill for binary existence check
- Use pointer return with NULL for not exists: Violates library's explicit result pattern

### Decision 2: Separate Functions for Files vs Directories
**Choice:** Provide three distinct functions: `fun_file_exists()`, `fun_directory_exists()`, `fun_path_exists()`

**Rationale:**
- Callers typically care about specific path types (file vs directory)
- `fun_path_exists()` serves as generic "does anything exist at this path"
- Matches common filesystem API patterns (Python's os.path.isfile/isdir)
- Single function with type parameter would be less readable

**Alternatives considered:**
- Single `fun_path_exists(path, type)` function: Less clear at call sites
- Only `fun_path_exists()` returning enum: Forces callers to check type

### Decision 3: Platform Implementation Strategy
**Choice:** Use `GetFileAttributes()` on Windows and `stat()` on POSIX

**Rationale:**
- Both APIs already used in existing directory.c implementations
- Minimal additional platform-specific code
- Efficient - single system call per check
- Well-understood, stable APIs across OS versions

**Alternatives considered:**
- Windows: `CreateFile()`: Overkill, requires handle cleanup
- POSIX: `access()`: Deprecated in favor of `stat()` family
- Use existing directory.c helper functions: Would require refactoring

## Risks / Trade-offs

**[Race conditions]** → File may be created/deleted between check and use
- **Mitigation:** Document that existence checks are point-in-time; callers should handle TOCTOU errors in subsequent operations

**[Symbolic links]** → Behavior with symlinks may differ across platforms
- **Mitigation:** Windows `GetFileAttributes()` follows symlinks; POSIX `stat()` follows symlinks. Document this behavior. If lstat-style behavior is needed, add separate functions later

**[Permission denied]** → Path exists but inaccessible returns "not exists" semantically
- **Mitigation:** Error code in result indicates permission issues; callers can distinguish "doesn't exist" from "can't check"

**[Performance]** → Multiple existence checks in loops could be slow
- **Mitigation:** For bulk operations, recommend directory listing instead of repeated existence checks

## Migration Plan

**Phase 1: Implementation**
1. Add function declarations to `include/filesystem/filesystem.h`
2. Add platform-specific implementations to `arch/filesystem/windows-amd64/directory.c`
3. Add platform-specific implementations to `arch/filesystem/linux-amd64/directory.c`
4. No rollback needed - additive change only

**Phase 2: Testing**
1. Create `tests/file_exists/` with build scripts
2. Test file existence (exists, doesn't exist, is directory)
3. Test directory existence (exists, doesn't exist, is file)
4. Test generic path existence
5. Test error conditions (invalid paths, permissions)

**Phase 3: Documentation**
1. Update filesystem.h with function documentation
2. Add usage examples to each function's doc comments

No migration needed for existing code - this is purely additive.

## Open Questions

None - implementation approach is clear from existing patterns.
