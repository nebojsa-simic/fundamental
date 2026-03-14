## Context

The Fundamental Library currently handles paths as platform-specific separator-delimited strings (`String` = `const char *`). This approach requires:
- `fun_path_separator()` to determine correct separator at runtime
- String parsing/splitting on every path operation
- Platform-specific handling in string manipulation functions
- Potential bugs when separators are mixed or mishandled

The library uses a component-based architecture with:
- Public API in `include/*/` directories
- Platform-specific implementations in `arch/<platform>/`
- Result types defined in `error/error.h` with `DEFINE_RESULT_TYPE` macros

This refactoring affects the entire filesystem module and potentially future file I/O operations.

## Goals / Non-Goals

**Goals:**
- Introduce `Path` type as array of string components (no separators in representation)
- Provide conversion functions: `fun_path_from_string()` and `fun_path_to_string()` at I/O boundaries
- Retain `fun_path_separator()` for manual path string construction use cases
- Eliminate separator-related logic from path manipulation functions
- Maintain cross-platform compatibility - paths work identically on all OS
- Support both absolute and relative paths
- Enable path operations (join, normalize, parent, filename) without string parsing

**Non-Goals:**
- No changes to file content I/O operations (future change)
- No symbolic link resolution logic
- No path validation beyond structural correctness
- No performance optimization for hot paths (focus on correctness first)
- No changes to memory allocation patterns (caller-allocated for Path components)

## Decisions

### Decision 1: Path Type Structure
**Choice:** `Path` is a struct containing array of `String` components with count, and `OutputPath` for output parameters

```c
typedef struct {
    const char **components;  // Array of path component strings
    size_t count;             // Number of components
    bool is_absolute;         // Whether path is absolute
} Path;

typedef Path *OutputPath;  // Output parameter for Path functions
```

**Rationale:**
- Array representation eliminates separator concerns entirely
- `is_absolute` flag preserves leading `/` or drive letter semantics
- Struct wrapper provides type safety vs raw array
- `OutputPath` typedef follows library convention (like `OutputString`)
- Immutable component strings (`const char *`) match library patterns
- Count enables O(1) length checks, iteration bounds

**Alternatives considered:**
- Linked list of components: More complex, worse cache locality
- Single allocated buffer with offsets: Requires parsing, defeats purpose
- Union for absolute/relative: Unnecessary complexity, `is_absolute` flag suffices

### Decision 2: Memory Ownership
**Choice:** Caller allocates and owns all Path memory including component array

**Rationale:**
- Consistent with library's existing memory patterns (Memory type, caller-allocated)
- No hidden allocations in Path functions
- Clear ownership - caller frees when done
- Avoids allocation overhead for temporary path operations
- Enables stack allocation for simple cases

**Implications:**
- Functions accept `Path` for input, `OutputPath` for output (avoid copying)
- Caller must allocate `components` array before calling functions
- No `fun_path_free()` - caller manages memory
- Conversion functions require pre-allocated output buffers

**Alternatives considered:**
- Library-allocated Paths: Requires `fun_path_free()`, inconsistent with Memory pattern
- Hybrid (library allocates components): Unclear ownership, error-prone

### Decision 3: Platform String Conversion at Boundary Only
**Choice:** Path→string conversion happens only at OS call boundaries in `arch/*/`

**Rationale:**
- Library code never sees platform-specific separators
- Architecture layer handles native string construction
- Minimizes conversion overhead (convert once per syscall, not per operation)
- Keeps platform-specific logic isolated in `arch/` directories

**Implementation:**
- `arch/filesystem/windows-amd64/path.c`: `path_to_native_string(Path, OutputString, wchar_t *)`
- `arch/filesystem/linux-amd64/path.c`: `path_to_native_string(Path, OutputString)`
- Functions construct separator-delimited string for POSIX `open()`, Windows `CreateFile()`, etc.

**Alternatives considered:**
- Convert at library API boundary: Loses Path benefits in internal code
- Pass components individually to syscalls: Not all APIs support this (e.g., `open()` needs single string)

### Decision 4: Path Join as Component Concatenation
**Choice:** `fun_path_join(Path base, Path relative, OutputPath output)` concatenates component arrays

**Rationale:**
- No string parsing or separator insertion logic
- Handle `..` and `.` at component level during join
- Output can be normalized in single pass
- Simpler than string-based join with separator handling

**Algorithm:**
1. Copy base components to output
2. For each relative component:
   - If `.` - skip (current directory)
   - If `..` - remove last output component (if any)
   - Otherwise - append component
3. Preserve `is_absolute` from base

### Decision 5: Migration Strategy
**Choice:** Single breaking change - no dual-API compatibility layer

**Rationale:**
- Compatibility layer doubles maintenance burden
- Clear signal to users that migration is required
- Library is pre-1.0 - breaking changes acceptable
- Migration is mechanical (wrap strings in Path at boundaries)

**Migration approach:**
1. Update all internal library code to use Path
2. Provide string→Path conversion utilities for callers
3. Update tests to use Path type
4. Document migration pattern in changelog

## Risks / Trade-offs

**[Performance overhead for simple paths]** → Array allocation may be slower than string ops for trivial cases
- **Mitigation:** Stack-allocated Path structs for common cases; benchmark critical paths; document that benefits outweigh costs for complex operations

**[Memory fragmentation]** → Many small allocations for path component arrays
- **Mitigation:** Caller can pool allocations; single contiguous allocation for components array; arena allocation pattern for batch operations

**[API migration friction]** → Breaking change requires all callers to update
- **Mitigation:** Clear migration guide; conversion helpers for string literals; mechanical transformation (wrap strings at boundaries)

**[Component lifetime management]** → Caller must ensure component strings outlive Path usage
- **Mitigation:** Document ownership clearly; use const pointers to signal immutability; provide helper to duplicate components if needed

**[Windows drive letters]** → Absolute paths on Windows include drive letter (C:, D:)
- **Mitigation:** Store drive letter as first component for Windows; abstract behind `is_absolute` flag; platform-specific conversion handles reconstruction

## Migration Plan

**Phase 1: Path Type Foundation**
1. Define `Path` struct in new `include/filesystem/path.h`
2. Implement conversion: `fun_path_from_string()`, `fun_path_to_string()`
3. Add component access: `fun_path_get_component()`, `fun_path_component_count()`
4. Create basic operations: `fun_path_join()`, `fun_path_normalize()`

**Phase 2: Filesystem API Migration**
1. Update `include/filesystem/filesystem.h` to use Path type
2. Update all filesystem function signatures
3. Update architecture layer to convert Path→native strings for syscalls
4. Retain `fun_path_separator()` for manual string construction scenarios

**Phase 3: Test Migration**
1. Update existing filesystem tests to use Path type
2. Add tests for Path conversion edge cases
3. Add tests for Path operations (join, normalize, parent, filename)
4. Verify cross-platform behavior

**Phase 4: Documentation**
1. Update API documentation with Path examples
2. Add migration guide for existing users
3. Document Path best practices

**Rollback Strategy:**
- Not feasible mid-implementation - breaking change is all-or-nothing
- Maintain git branch until Phase 3 tests pass
- If critical issues found, revert entire change and address before retry

## Open Questions

1. **Maximum path components**: Should we enforce a limit (e.g., 256 components) or allow arbitrary length? Trade-off: safety vs flexibility.

2. **Empty components**: How to handle `//` or trailing `/`? Options: collapse to single, preserve as empty string, reject as invalid. Recommendation: collapse during parsing.

3. **Unicode handling**: Path components may contain non-ASCII characters. Should Path store UTF-8 and let platform layer handle conversion, or normalize at parse time? Recommendation: store as UTF-8, convert at boundary.
