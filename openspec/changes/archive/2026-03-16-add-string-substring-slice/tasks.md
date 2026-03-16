## 1. Header File Updates

- [x] 1.1 Add `fun_string_substring()` declaration to `include/string/string.h`
- [x] 1.2 Add `fun_string_slice()` declaration to `include/string/string.h`
- [x] 1.3 Add documentation with examples for both functions
- [x] 1.4 Add `ERROR_CODE_INDEX_OUT_OF_BOUNDS` to error codes if not already present

## 2. Implementation

- [x] 2.1 Implement `fun_string_substring()` in `src/string/stringOperations.c`
- [x] 2.2 Add NULL parameter validation for substring function
- [x] 2.3 Add bounds checking for start index and length
- [x] 2.4 Implement `fun_string_slice()` in `src/string/stringOperations.c`
- [x] 2.5 Add negative index resolution for slice function
- [x] 2.6 Handle edge case: start >= end returns empty string
- [x] 2.7 Add buffer size validation for both functions

## 3. Error Handling

- [x] 3.1 Return `ERROR_CODE_NULL_POINTER` for NULL parameters
- [x] 3.2 Return `ERROR_CODE_INDEX_OUT_OF_BOUNDS` for invalid indices
- [x] 3.3 Return `ERROR_CODE_BUFFER_TOO_SMALL` when output buffer insufficient
- [x] 3.4 Ensure null termination on all successful operations

## 4. Test Implementation

- [x] 4.1 Create `tests/stringSubstring/` directory
- [x] 4.2 Create `test_substring.c` with substring extraction tests
- [x] 4.3 Create `test_slice.c` with slice operation tests
- [x] 4.4 Add tests for negative indices in slice function
- [x] 4.5 Add tests for error conditions (NULL, bounds, buffer size)
- [x] 4.6 Create `build-windows-amd64.bat` for test compilation
- [x] 4.7 Create `build-linux-amd64.sh` for test compilation

## 5. Verification

- [x] 5.1 Run all substring tests and verify pass
- [x] 5.2 Run all slice tests and verify pass
- [x] 5.3 Run existing string module tests to verify no regressions
- [x] 5.4 Update change tasks.md to mark all tasks complete
