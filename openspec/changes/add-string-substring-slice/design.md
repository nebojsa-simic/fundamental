## Context

The Fundamental Library string module provides safe string manipulation with functions for conversion, comparison, joining, copying, and templating. However, it lacks substring extraction capabilities, which are fundamental operations found in most string libraries (e.g., `substr`, `slice`, `substring` in JavaScript, Python, etc.).

Current string module pattern:
- Output buffers are caller-allocated (`OutputString` = `char *`)
- Functions return `ErrorResult` for error handling
- NULL parameter validation with `ERROR_CODE_NULL_POINTER`
- Buffer overflow protection with size parameters

## Goals / Non-Goals

**Goals:**
- Add substring extraction with start index and length parameters
- Add slice operation with start and end indices (Python-style)
- Follow existing string module conventions for error handling and memory
- Provide comprehensive bounds checking and validation
- Maintain consistency with library patterns (caller-allocated buffers)

**Non-Goals:**
- No new memory allocation patterns (caller provides output buffer)
- No Unicode/multi-byte character handling (operates on byte indices)
- No mutable string operations (returns new string in provided buffer)
- No changes to existing string functions

## Decisions

### Decision 1: Two separate functions (substring vs slice)
**Choice:** Provide both `fun_string_substring(start, length)` and `fun_string_slice(start, end)`

**Rationale:**
- `substring(start, length)` - C-style, explicit length control, matches existing library patterns
- `slice(start, end)` - Python-style, more intuitive for "from X to Y" operations
- Both are common patterns developers expect

**Alternatives considered:**
- Single function with optional end parameter: Rejected - C doesn't support optional parameters cleanly
- Only one variant: Rejected - both patterns have legitimate use cases

### Decision 2: Index bounds handling
**Choice:** Return `ERROR_CODE_INDEX_OUT_OF_BOUNDS` for invalid indices, do not auto-clamp

**Rationale:**
- Explicit errors help catch bugs early
- Consistent with library's error-first approach
- Auto-clamping can hide logic errors

**Alternatives considered:**
- Clamp to valid range: Rejected - silent failures are harder to debug
- Return empty string on bounds error: Rejected - loses error information

### Decision 3: Negative index handling for slice
**Choice:** Support negative indices for `slice()` only (Python-style), treat as offset from end

**Rationale:**
- Python-style negative indices are a key benefit of `slice()` over `substring()`
- `substring()` uses explicit length, so negative indices less meaningful
- Clear semantic difference between the two functions

### Decision 4: Function signatures
**Choice:**
```c
ErrorResult fun_string_substring(String source, size_t start, size_t length, OutputString output, size_t output_size);
ErrorResult fun_string_slice(String source, size_t start, size_t end, OutputString output, size_t output_size);
```

**Rationale:**
- Consistent with existing string functions (all take `output_size` for safety)
- `size_t` for indices (matches `fun_string_length` return type)
- `String` (const char*) for input, `OutputString` (char*) for output

## Risks / Trade-offs

**[Risk]** Negative index handling adds complexity to slice implementation
→ **Mitigation:** Clear documentation, comprehensive tests for negative index cases

**[Risk]** Two similar functions may confuse developers about which to use
→ **Mitigation:** Clear documentation with use case examples for each

**[Trade-off]** No auto-clamping means more defensive calling code needed
→ **Acceptable:** Explicit errors are preferred over silent data corruption

**[Risk]** Buffer size calculation burden on caller
→ **Mitigation:** Document that output_size must be >= (extracted length + 1) for null terminator
