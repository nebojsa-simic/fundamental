## 1. Path Type Foundation

- [ ] 1.1 Create new header file `include/filesystem/path.h` with Path struct definition
- [ ] 1.2 Define Path struct with components array, count, and is_absolute fields
- [ ] 1.3 Define OutputPath as `typedef Path *OutputPath` to follow library conventions
- [ ] 1.4 Define Path-related types: PathComponent, PathResult for error handling
- [ ] 1.5 Add forward declarations for all Path functions in path.h

## 2. String to Path Conversion

- [ ] 2.1 Implement `fun_path_from_string(String path, OutputPath output)` in `src/filesystem/path.c`
- [ ] 2.2 Handle absolute path detection (leading `/` or Windows drive letter)
- [ ] 2.3 Parse string by separator, populate components array
- [ ] 2.4 Handle edge cases: empty string, NULL input, trailing separators, multiple separators
- [ ] 2.5 Preserve `.` and `..` components without normalization (separate function)
- [ ] 2.6 Add error handling with appropriate error codes

## 3. Path to String Conversion

- [ ] 3.1 Implement `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` in `src/filesystem/path.c`
- [ ] 3.2 Add platform-specific separator logic in arch layer
- [ ] 3.3 Implement `arch/filesystem/windows-amd64/path.c` with Windows separator handling
- [ ] 3.4 Implement `arch/filesystem/linux-amd64/path.c` with POSIX separator handling
- [ ] 3.5 Handle absolute path prefix (add leading separator or drive letter)
- [ ] 3.6 Validate buffer size, return error if too small

## 4. Path Operations - Join

- [ ] 4.1 Implement `fun_path_join(Path base, Path relative, OutputPath output)` in `src/filesystem/path.c`
- [ ] 4.2 Copy base components to output
- [ ] 4.3 Process relative components: skip `.`, handle `..` by removing last base component
- [ ] 4.4 Preserve is_absolute flag from base
- [ ] 4.5 Handle edge cases: empty base, empty relative

## 5. Path Operations - Normalize

- [ ] 5.1 Implement `fun_path_normalize(Path path, OutputPath output)` in `src/filesystem/path.c`
- [ ] 5.2 Remove all `.` components
- [ ] 5.3 Resolve `..` by removing preceding component
- [ ] 5.4 Discard `..` that would go before root for absolute paths
- [ ] 5.5 Preserve is_absolute flag

## 6. Path Operations - Parent and Filename

- [ ] 6.1 Implement `fun_path_get_parent(Path path, OutputPath output)` in `src/filesystem/path.c`
- [ ] 6.2 Copy all components except last to output
- [ ] 6.3 Handle edge cases: single component, empty path, root path
- [ ] 6.4 Implement `fun_path_get_filename(Path path, OutputPath output)` in `src/filesystem/path.c`
- [ ] 6.5 Copy only last component to output
- [ ] 6.6 Handle edge cases: empty path, single component

## 7. Path Component Access

- [ ] 7.1 Implement `fun_path_get_component(Path path, size_t index)` in `src/filesystem/path.c`
- [ ] 7.2 Return component string at index, NULL if out of bounds
- [ ] 7.3 Implement `fun_path_component_count(Path path)` as inline or simple function
- [ ] 7.4 Implement `fun_path_is_valid(Path path)` validation function
- [ ] 7.5 Check for NULL components, empty components

## 8. Filesystem API Migration

- [ ] 8.1 Update `include/filesystem/filesystem.h` to include path.h
- [ ] 8.2 Change `fun_filesystem_create_directory(String)` to `fun_filesystem_create_directory(Path)`
- [ ] 8.3 Change `fun_filesystem_remove_directory(String)` to `fun_filesystem_remove_directory(Path)`
- [ ] 8.4 Change `fun_filesystem_list_directory(String, Memory)` to use Path parameter
- [ ] 8.5 Update `fun_path_join` signature to use Path types instead of String
- [ ] 8.6 Update `fun_path_normalize` signature to use Path and OutputPath types
- [ ] 8.7 Update `fun_path_get_parent` signature to use Path and OutputPath types
- [ ] 8.8 Update `fun_path_get_filename` signature to use Path and OutputPath types
- [ ] 8.9 Add `fun_path_from_string(String path, OutputPath output)` declaration to filesystem.h
- [ ] 8.10 Add `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` declaration to filesystem.h
- [ ] 8.11 Retain `fun_path_separator()` function declaration - keep for manual string construction
- [ ] 8.12 Update all function documentation with Path examples

## 9. Architecture Layer - Platform Conversion

- [ ] 9.1 Update `arch/filesystem/windows-amd64/directory.c` to accept Path parameters
- [ ] 9.2 Add internal function to convert Path to Windows wide string for API calls
- [ ] 9.3 Update `arch/filesystem/linux-amd64/directory.c` to accept Path parameters
- [ ] 9.4 Add internal function to convert Path to POSIX string for syscalls
- [ ] 9.5 Update all internal filesystem functions to use Path→native conversion at syscall boundary

## 10. Test Implementation

- [ ] 10.1 Create `tests/path_type/` directory structure
- [ ] 10.2 Create `test_path_conversion.c` with string→Path and Path→string tests
- [ ] 10.3 Create `test_path_operations.c` with join, normalize, parent, filename tests
- [ ] 10.4 Create `test_path_component_access.c` with component access tests
- [ ] 10.5 Create `tests/path_type/build-windows-amd64.bat` following existing patterns
- [ ] 10.6 Create `tests/path_type/build-linux-amd64.sh` following existing patterns
- [ ] 10.7 Update `tests/filesystem/test_filesystem.c` to use Path type
- [ ] 10.8 Add tests for edge cases: empty paths, root paths, invalid inputs

## 11. Migration and Cleanup

- [ ] 11.1 Retain uses of `fun_path_separator()` for manual path construction scenarios
- [ ] 11.2 Update any other modules that include filesystem.h and use path functions
- [ ] 11.3 Verify no String path parameters remain in filesystem API (except conversion functions)
- [ ] 11.4 Update any internal code that constructs paths manually

## 12. Documentation

- [ ] 12.1 Add comprehensive examples to path.h documentation
- [ ] 12.2 Document memory ownership pattern (caller-allocated components)
- [ ] 12.3 Add migration guide section to filesystem documentation
- [ ] 12.4 Update README or CHANGELOG with breaking change notice
