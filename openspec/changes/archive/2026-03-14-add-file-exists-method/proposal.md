## Why

Developers need to check if a file exists before performing operations on it, but the library currently lacks a dedicated file existence check function. This forces users to attempt file operations and handle errors, which is inefficient and unclear for simple existence checks.

## What Changes

- Add `fun_file_exists(String path)` function to check if a file exists
- Add `fun_directory_exists(String path)` function to check if a directory exists
- Add `fun_path_exists(String path)` function to check if any path (file or directory) exists
- Add `FILE_EXISTS`, `FILE_NOT_EXISTS`, `FILE_IS_DIRECTORY` return codes to error module
- No breaking changes - purely additive functionality

## Capabilities

### New Capabilities
- `file-exists`: File and directory existence checking functionality

### Modified Capabilities

## Impact

- **New header functions**: `filesystem.h` will export three new existence-checking functions
- **Platform-specific implementations**: Required in `arch/*/filesystem.c` for Windows (GetFileAttributes) and POSIX (stat)
- **Error module**: May need new error codes for file existence states
- **Tests**: New test directory `tests/file_exists/` to validate existence checks across platforms
- **Documentation**: Update filesystem module documentation to include existence functions
