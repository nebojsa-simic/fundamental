# Filesystem Module Specification

## Purpose
Filesystem module provides directory operations and path manipulation utilities for cross-platform file system interactions without stdlib dependencies.

## ADDED Requirements

### Requirement: Directory Creation
The filesystem module SHALL provide function to create directories including parent directories.

#### Scenario: Create new directory
- **WHEN** fun_filesystem_create_directory(path) is called with valid path
- **THEN** directory is created at specified path
- **AND** ErrorResult with ERROR_CODE_OK is returned on success
- **AND** all parent directories are created if they don't exist

#### Scenario: Handle existing directory
- **WHEN** fun_filesystem_create_directory is called with path to existing directory
- **THEN** function succeeds silently (idempotent operation)
- **AND** ErrorResult with ERROR_CODE_OK is returned

#### Scenario: Handle invalid path
- **WHEN** fun_filesystem_create_directory is called with invalid path (null, empty, invalid characters)
- **THEN** ErrorResult with ERROR_CODE_PATH_INVALID is returned
- **AND** no directory is created

#### Scenario: Handle permission denied
- **WHEN** fun_filesystem_create_directory is called without write permission
- **THEN** ErrorResult with ERROR_CODE_PERMISSION_DENIED is returned
- **AND** no directory is created

### Requirement: Directory Removal
The filesystem module SHALL provide function to remove empty directories.

#### Scenario: Remove empty directory
- **WHEN** fun_filesystem_remove_directory(path) is called with path to empty directory
- **THEN** directory is removed from filesystem
- **AND** ErrorResult with ERROR_CODE_OK is returned

#### Scenario: Handle non-empty directory
- **WHEN** fun_filesystem_remove_directory is called with path to non-empty directory
- **THEN** ErrorResult with ERROR_CODE_DIRECTORY_NOT_EMPTY is returned
- **AND** directory and its contents remain unchanged

#### Scenario: Handle non-existent directory
- **WHEN** fun_filesystem_remove_directory is called with path to non-existent directory
- **THEN** ErrorResult with ERROR_CODE_DIRECTORY_NOT_FOUND is returned

#### Scenario: Handle file path instead of directory
- **WHEN** fun_filesystem_remove_directory is called with path to file (not directory)
- **THEN** ErrorResult with ERROR_CODE_NOT_DIRECTORY is returned
- **AND** file remains unchanged

### Requirement: Directory Listing
The filesystem module SHALL provide function to list directory contents.

#### Scenario: List directory contents
- **WHEN** fun_filesystem_list_directory(path, output) is called with valid directory path
- **THEN** output buffer is filled with directory entry names
- **AND** each entry name is null-terminated
- **AND** entries are separated by newlines or null bytes
- **AND** ErrorResult with ERROR_CODE_OK is returned

#### Scenario: Handle empty directory
- **WHEN** fun_filesystem_list_directory is called on empty directory
- **THEN** output buffer remains empty (zero bytes written)
- **AND** ErrorResult with ERROR_CODE_OK is returned

#### Scenario: Handle non-directory path
- **WHEN** fun_filesystem_list_directory is called with path to file (not directory)
- **THEN** ErrorResult with ERROR_CODE_NOT_DIRECTORY is returned
- **AND** output buffer is not modified

#### Scenario: Handle insufficient buffer
- **WHEN** fun_filesystem_list_directory is called with output buffer too small
- **THEN** ErrorResult with ERROR_CODE_BUFFER_TOO_SMALL is returned
- **AND** required buffer size is indicated in error metadata if available

### Requirement: Path Join
The filesystem module SHALL provide function to join path components with platform-appropriate separators.

#### Scenario: Join two path components
- **WHEN** fun_path_join(base, relative) is called with valid paths
- **THEN** result contains base + separator + relative
- **AND** platform-appropriate separator is used (/ on POSIX, \ on Windows)
- **AND** redundant separators are avoided

#### Scenario: Join with absolute relative path
- **WHEN** fun_path_join is called with absolute path as relative component
- **THEN** absolute path is returned unchanged (relative overrides base)

#### Scenario: Join with empty base
- **WHEN** fun_path_join is called with empty base path
- **THEN** relative path is returned unchanged

### Requirement: Path Normalization
The filesystem module SHALL provide function to normalize paths by resolving . and .. components.

#### Scenario: Normalize path with current directory markers
- **WHEN** fun_path_normalize(path) is called with path containing "." components
- **THEN** "." components are removed from result
- **AND** path semantics are preserved

#### Scenario: Normalize path with parent directory markers
- **WHEN** fun_path_normalize is called with path containing ".." components
- **THEN** ".." components resolve to parent directories
- **AND** result does not contain ".." unless path goes above root

#### Scenario: Normalize path with redundant separators
- **WHEN** fun_path_normalize is called with multiple consecutive separators
- **THEN** redundant separators are collapsed to single separator

### Requirement: Path Parent Extraction
The filesystem module SHALL provide function to extract parent directory from path.

#### Scenario: Get parent of nested path
- **WHEN** fun_path_get_parent(path) is called with nested path
- **THEN** result contains all components except the last
- **AND** trailing separator is removed if present

#### Scenario: Get parent of root path
- **WHEN** fun_path_get_parent is called with root path (/ or C:\)
- **THEN** root path is returned unchanged

#### Scenario: Get parent of single component
- **WHEN** fun_path_get_parent is called with single component (no separators)
- **THEN** empty string or "." is returned

### Requirement: Path Filename Extraction
The filesystem module SHALL provide function to extract filename component from path.

#### Scenario: Get filename from nested path
- **WHEN** fun_path_get_filename(path) is called with nested path
- **THEN** result contains only the last component after final separator
- **AND** trailing separators are ignored

#### Scenario: Get filename from path with extension
- **WHEN** fun_path_get_filename is called with file.ext
- **THEN** result is "file.ext" (extension included)

#### Scenario: Get filename from directory path
- **WHEN** fun_path_get_filename is called with directory path ending in separator
- **THEN** empty string is returned (no filename component)

## Constraints
- All paths use String type (const char* wrapper)
- Caller-allocated memory pattern for all output buffers
- Platform-appropriate path separators used internally
- Functions are synchronous (no async operations)
- No stdlib runtime functions used in implementation
- Error codes are positive integers with ERROR_CODE_* constants
- Directory removal only succeeds on empty directories (no recursive delete)
