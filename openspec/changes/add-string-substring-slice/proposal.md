## Why

The string module currently lacks substring extraction and slice operations, which are fundamental string manipulation capabilities. While `fun_string_index_of` exists for finding substrings, there is no way to extract a portion of a string. This forces developers to manually copy characters or use error-prone pointer arithmetic, undermining the library's safety guarantees.

## What Changes

- Add `fun_string_substring(String source, size_t start, size_t length, OutputString output)` for extracting substrings
- Add `fun_string_slice(String source, size_t start, size_t end, OutputString output)` for Python-style slicing
- Add bounds validation and error handling for out-of-range operations
- Update string module specification to include substring/slice requirements
- Add comprehensive tests for substring and slice operations

## Capabilities

### New Capabilities
<!-- None - this extends existing string module capabilities -->

### Modified Capabilities
<!-- Existing capabilities whose REQUIREMENTS are changing -->
- `string`: Add substring extraction and slice requirements to existing string specification

## Impact

- **Modified Files**: `include/string/string.h`, `src/string/stringOperations.c`
- **New Tests**: `tests/stringSubstring/` test directory
- **Dependencies**: None (uses existing memory and error handling patterns)
- **API Surface**: Two new public functions added to string module
