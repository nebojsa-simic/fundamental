# file-exists Specification

## Purpose
TBD - created by archiving change add-file-exists-method. Update Purpose after archive.
## Requirements
### Requirement: File Existence Check
The filesystem module SHALL provide a function to check if a file exists at the specified path.

#### Scenario: File exists at path
- **WHEN** `fun_file_exists(path)` is called with a path to an existing file
- **THEN** the function returns a `boolResult` with `value = true` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: File does not exist
- **WHEN** `fun_file_exists(path)` is called with a path where no file exists
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Path is a directory, not a file
- **WHEN** `fun_file_exists(path)` is called with a path to an existing directory
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Invalid path format
- **WHEN** `fun_file_exists(path)` is called with a NULL or invalid path
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NULL_POINTER` or `ERROR_CODE_PATH_INVALID`

#### Scenario: Permission denied
- **WHEN** `fun_file_exists(path)` is called with a path to a file without read permissions
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_PERMISSION_DENIED`

### Requirement: Directory Existence Check
The filesystem module SHALL provide a function to check if a directory exists at the specified path.

#### Scenario: Directory exists at path
- **WHEN** `fun_directory_exists(path)` is called with a path to an existing directory
- **THEN** the function returns a `boolResult` with `value = true` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Directory does not exist
- **WHEN** `fun_directory_exists(path)` is called with a path where no directory exists
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Path is a file, not a directory
- **WHEN** `fun_directory_exists(path)` is called with a path to an existing file
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Invalid path format
- **WHEN** `fun_directory_exists(path)` is called with a NULL or invalid path
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NULL_POINTER` or `ERROR_CODE_PATH_INVALID`

#### Scenario: Permission denied
- **WHEN** `fun_directory_exists(path)` is called with a path to a directory without read permissions
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_PERMISSION_DENIED`

### Requirement: Generic Path Existence Check
The filesystem module SHALL provide a function to check if any path (file or directory) exists at the specified path.

#### Scenario: Path exists as file
- **WHEN** `fun_path_exists(path)` is called with a path to an existing file
- **THEN** the function returns a `boolResult` with `value = true` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Path exists as directory
- **WHEN** `fun_path_exists(path)` is called with a path to an existing directory
- **THEN** the function returns a `boolResult` with `value = true` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Path does not exist
- **WHEN** `fun_path_exists(path)` is called with a path where nothing exists
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NO_ERROR`

#### Scenario: Invalid path format
- **WHEN** `fun_path_exists(path)` is called with a NULL or invalid path
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_NULL_POINTER` or `ERROR_CODE_PATH_INVALID`

#### Scenario: Permission denied
- **WHEN** `fun_path_exists(path)` is called with a path without read permissions
- **THEN** the function returns a `boolResult` with `value = false` and `error.code = ERROR_CODE_PERMISSION_DENIED`

### Requirement: Path Type Detection
The filesystem module SHALL correctly distinguish between files, directories, and non-existent paths.

#### Scenario: Symlink to file (platform-dependent)
- **WHEN** `fun_file_exists(path)` is called with a path to a symlink pointing to a file
- **THEN** the function follows the symlink and returns `value = true` (on platforms that support symlinks)

#### Scenario: Symlink to directory (platform-dependent)
- **WHEN** `fun_directory_exists(path)` is called with a path to a symlink pointing to a directory
- **THEN** the function follows the symlink and returns `value = true` (on platforms that support symlinks)

### Requirement: Cross-Platform Behavior
The filesystem module SHALL provide consistent existence check behavior across Windows and POSIX platforms.

#### Scenario: Windows path separators
- **WHEN** existence check functions are called with Windows-style paths using backslashes
- **THEN** the functions correctly resolve and check the path

#### Scenario: POSIX path separators
- **WHEN** existence check functions are called with POSIX-style paths using forward slashes
- **THEN** the functions correctly resolve and check the path

#### Scenario: Relative paths
- **WHEN** existence check functions are called with relative paths
- **THEN** the functions resolve paths relative to the current working directory

#### Scenario: Absolute paths
- **WHEN** existence check functions are called with absolute paths
- **THEN** the functions resolve paths from the filesystem root

