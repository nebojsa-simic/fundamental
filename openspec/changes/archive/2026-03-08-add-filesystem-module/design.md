## Context

The Fundamental library provides file I/O operations (read, write, append) but lacks directory management and path manipulation capabilities. Current state: no directory creation, no directory listing, no path utilities. Applications must rely on stdlib functions or platform-specific code for these operations, violating the zero-stdlib-runtime design principle.

This design adds filesystem operations following existing library patterns: public headers in `include/filesystem/`, implementation in `src/filesystem/`, and platform abstractions in `arch/filesystem/`.

## Goals / Non-Goals

**Goals:**
- Provide `fun_filesystem_create_directory()` for creating directories
- Provide `fun_filesystem_list_directory()` for listing directory contents
- Provide `fun_filesystem_remove_directory()` for removing empty directories
- Provide path utilities: `fun_path_join()`, `fun_path_normalize()`, `fun_path_get_parent()`, `fun_path_get_filename()`
- Platform abstraction (Windows API, POSIX syscalls)
- Zero stdlib runtime functions (headers OK for types, no runtime calls)
- Follow existing error handling patterns with ErrorResult
- String type for all path operations (const char* wrapper)

**Non-Goals:**
- Directory iteration with handles (open/read/close pattern) - use simple list for now
- File metadata retrieval (size, timestamps, permissions) - separate capability
- Symbolic link handling - platform complexity, defer
- Recursive directory operations (remove tree) - can build on top later
- Path expansion (~, environment variables) - higher-level feature
- Thread safety - single-threaded usage initially

## Decisions

### 1. Directory Creation API
**Decision:** `fun_filesystem_create_directory(String path)` returns ErrorResult

**Rationale:** Simple, synchronous operation. ErrorResult for consistency with library patterns. Creates parent directories if they don't exist (mkdir -p behavior).

**Alternatives Considered:**
- `fun_filesystem_mkdir()` - Less clear intent, POSIX naming
- Async return type - Directory creation is fast, no need for async complexity
- Option to create parents or fail - Keep simple, always create parents

### 2. Directory Removal API
**Decision:** `fun_filesystem_remove_directory(String path)` returns ErrorResult

**Rationale:** Simple, synchronous operation for removing empty directories only. ErrorResult for consistency. Only removes if directory is empty (rmdir behavior, not rm -rf).

**Alternatives Considered:**
- `fun_filesystem_rmdir()` - Less clear intent, POSIX naming
- Recursive remove (rm -rf) - Dangerous, can delete entire trees accidentally
- Async return type - Directory removal is fast, no need for async complexity

### 3. Directory Listing Strategy
**Decision:** `fun_filesystem_list_directory(String path, Memory output)` returns ErrorResult with JSON-like array of strings

**Rationale:** Caller pre-allocates buffer, function fills with directory entries. Consistent with file I/O pattern (Read/Write structs). JSON-like format easy to parse.

**Alternatives Considered:**
- Callback-based iteration - More complex API, harder to use
- Return array of String structs - Requires dynamic allocation, memory management complexity
- Directory handle with read_next - More flexible but adds state management

### 4. Path Utilities Design
**Decision:** Pure functions operating on String types, return String or ErrorResult

**Rationale:** No state, easy to test and compose. Path operations are string manipulations, no platform-specific logic needed (except separator handling).

**Alternatives Considered:**
- Path struct with methods - More idiomatic but adds new type complexity
- In-place modification - Violates caller-allocated memory pattern
- Platform-specific path handling - Paths should work cross-platform

### 5. Path Separator Handling
**Decision:** Accept both forward slash (/) and backslash (\) on all platforms, normalize to platform separator internally

**Rationale:** User-friendly, works regardless of user's platform. Windows accepts forward slashes in most APIs. Normalize for consistent output.

**Alternatives Considered:**
- Reject wrong separators - Frustrating for users
- Keep user's separator - Inconsistent output, potential issues
- Always use forward slash - Windows APIs accept it, but non-standard for Windows

### 6. Error Handling
**Decision:** Return ErrorResult with specific error codes (DIRECTORY_EXISTS, PATH_INVALID, PERMISSION_DENIED, etc.)

**Rationale:** Consistent with library conventions. Callers can check specific conditions. Positive integer error codes.

**Alternatives Considered:**
- Boolean return with errno - Less type-safe, platform-specific
- Dedicated filesystem error type - Adds API surface without clear benefit
- Void return with internal logging - Hides failures from callers

## Risks / Trade-offs

**[Buffer overflow in directory listing]** → Caller must pre-allocate sufficient buffer. Mitigation: Document required buffer size calculation, return error if insufficient.

**[Path normalization edge cases]** → Paths with `..`, `.`, multiple separators, drive letters (Windows). Mitigation: Comprehensive test coverage, handle common cases first.

**[Windows long path support]** → Windows has MAX_PATH limitation (260 chars). Mitigation: Use Unicode APIs with `\\?\` prefix for long paths if needed later.

**[Directory listing order]** → No guaranteed order (alphabetic, by time, etc.). Mitigation: Document unspecified order, can add sorting option later.

**[Memory efficiency for large directories]** → Pre-allocation may waste memory for small directories or fail for large ones. Mitigation: Caller can estimate, future version could support streaming.

**[Race conditions in create_directory]** → Another process may create directory between check and creation. Mitigation: Platform APIs handle this atomically where possible.

**[Non-empty directory removal]** → Removing non-empty directory could cause data loss. Mitigation: Only remove empty directories, return error if directory contains entries.

## Migration Plan

Not applicable - this is a new capability with no existing API to migrate.

## Open Questions

- Should directory listing include `.` and `..` entries or filter them?
- Should `create_directory` fail if directory already exists or succeed silently?
- Is JSON-like format right for directory listing or should we use a simpler delimiter?
- Should path utilities handle relative vs absolute paths differently?
