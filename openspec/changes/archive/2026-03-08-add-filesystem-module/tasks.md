## 1. Header and API Definitions

- [x] 1.1 Create include/filesystem/filesystem.h with function declarations
- [x] 1.2 Define fun_filesystem_create_directory(String path) API
- [x] 1.3 Define fun_filesystem_list_directory(String path, Memory output) API
- [x] 1.4 Define fun_filesystem_remove_directory(String path) API
- [x] 1.5 Define fun_path_join(String base, String relative) API
- [x] 1.6 Define fun_path_normalize(String path) API
- [x] 1.7 Define fun_path_get_parent(String path) API
- [x] 1.8 Define fun_path_get_filename(String path) API
- [x] 1.9 Define error codes for filesystem operations (ERROR_CODE_DIRECTORY_*, ERROR_CODE_PATH_*)
- [x] 1.10 Add filesystem module to include directory structure

## 2. Directory Operations Implementation

- [x] 2.1 Create src/filesystem/directory.c with core logic
- [x] 2.2 Implement fun_filesystem_create_directory with parent directory creation
- [x] 2.3 Implement directory existence check helper
- [x] 2.4 Implement recursive parent directory creation
- [x] 2.5 Implement fun_filesystem_remove_directory for empty directories only
- [x] 2.6 Implement directory emptiness check helper
- [x] 2.7 Implement fun_filesystem_list_directory with buffer filling
- [x] 2.8 Implement directory entry enumeration helper
- [x] 2.9 Add NULL parameter validation for all functions
- [x] 2.10 Add error handling with appropriate error codes

## 3. Path Utilities Implementation

- [x] 3.1 Create src/filesystem/path.c with path manipulation functions
- [x] 3.2 Implement fun_path_join with separator handling
- [x] 3.3 Implement fun_path_normalize resolving . and .. components
- [x] 3.4 Implement redundant separator collapsing
- [x] 3.5 Implement fun_path_get_parent extracting parent component
- [x] 3.6 Implement fun_path_get_filename extracting filename component
- [x] 3.7 Handle edge cases (root paths, empty paths, single components)
- [x] 3.8 Add path validation helper function

## 4. Windows Platform Layer

- [x] 4.1 Create arch/filesystem/windows-amd64/directory.c
- [x] 4.2 Implement CreateDirectoryW for directory creation
- [x] 4.3 Implement RemoveDirectoryW for directory removal
- [x] 4.4 Implement FindFirstFileW/FindNextFileW for directory listing
- [x] 4.5 Handle Windows-specific error codes (GetLastError)
- [x] 4.6 Support long paths with \\?\ prefix if needed
- [x] 4.7 Convert between UTF-8 String and UTF-16 Windows APIs
- [x] 4.8 Create arch/filesystem/windows-amd64/path.c
- [x] 4.9 Implement backslash separator handling for Windows

## 5. POSIX Platform Layer

- [x] 5.1 Create arch/filesystem/linux-amd64/directory.c
- [x] 5.2 Implement mkdir with mode for directory creation
- [x] 5.3 Implement rmdir for directory removal
- [x] 5.4 Implement opendir/readdir/closedir for directory listing
- [x] 5.5 Handle POSIX-specific error codes (errno)
- [x] 5.6 Create arch/filesystem/linux-amd64/path.c
- [x] 5.7 Implement forward slash separator handling for POSIX

## 6. Error Handling and Codes

- [x] 6.1 Add filesystem error codes to include/error/error.h
- [x] 6.2 Define ERROR_CODE_DIRECTORY_EXISTS
- [x] 6.3 Define ERROR_CODE_DIRECTORY_NOT_FOUND
- [x] 6.4 Define ERROR_CODE_DIRECTORY_NOT_EMPTY
- [x] 6.5 Define ERROR_CODE_NOT_DIRECTORY
- [x] 6.5 Define ERROR_CODE_PATH_INVALID
- [x] 6.6 Define ERROR_CODE_PERMISSION_DENIED
- [x] 6.7 Define ERROR_CODE_BUFFER_TOO_SMALL
- [x] 6.8 Create error message mappings for filesystem errors

## 7. Tests

- [x] 7.1 Create tests/filesystem/ directory structure
- [x] 7.2 Create tests/filesystem/build-windows-amd64.bat
- [x] 7.3 Create tests/filesystem/build-linux-amd64.sh
- [x] 7.4 Implement test_filesystem_create_directory_basic (spec: Create new directory)
- [x] 7.5 Implement test_filesystem_create_directory_nested (spec: Create with parent directories)
- [x] 7.6 Implement test_filesystem_create_directory_exists (spec: Handle existing directory)
- [x] 7.7 Implement test_filesystem_create_directory_invalid_path (spec: Handle invalid path)
- [x] 7.8 Implement test_filesystem_remove_directory_basic (spec: Remove empty directory)
- [x] 7.9 Implement test_filesystem_remove_directory_non_empty (spec: Handle non-empty directory)
- [x] 7.10 Implement test_filesystem_remove_directory_not_found (spec: Handle non-existent directory)
- [x] 7.11 Implement test_filesystem_remove_directory_file_path (spec: Handle file path instead of directory)
- [x] 7.12 Implement test_filesystem_list_directory_basic (spec: List directory contents)
- [x] 7.13 Implement test_filesystem_list_directory_empty (spec: Handle empty directory)
- [x] 7.14 Implement test_filesystem_list_directory_file_path (spec: Handle non-directory path)
- [x] 7.15 Implement test_path_join_basic (spec: Join two path components)
- [x] 7.16 Implement test_path_join_absolute_relative (spec: Join with absolute relative path)
- [x] 7.17 Implement test_path_normalize_dot_components (spec: Normalize . components)
- [x] 7.18 Implement test_path_normalize_dotdot_components (spec: Normalize .. components)
- [x] 7.19 Implement test_path_normalize_redundant_separators (spec: Collapse redundant separators)
- [x] 7.20 Implement test_path_get_parent_nested (spec: Get parent of nested path)
- [x] 7.21 Implement test_path_get_parent_root (spec: Get parent of root path)
- [x] 7.22 Implement test_path_get_filename_basic (spec: Get filename from path)
- [x] 7.23 Implement test_path_get_filename_with_extension (spec: Get filename with extension)
- [x] 7.24 Implement test_filesystem_null_parameters (spec: NULL validation)

## 8. Documentation

- [x] 8.1 Add function documentation comments in filesystem.h
- [x] 8.2 Document error codes and their meanings
- [x] 8.3 Document buffer size requirements for directory listing
- [x] 8.4 Add usage examples for common filesystem patterns
- [x] 8.5 Document platform-specific considerations (path separators, encoding)
- [x] 8.6 Update README.md with filesystem module overview
