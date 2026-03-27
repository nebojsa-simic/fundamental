## MODIFIED Requirements

### Requirement: Directory Remove Operation
The filesystem module SHALL provide function to remove directories using Path type.

#### Scenario: Remove empty directory
- **WHEN** `fun_filesystem_remove_directory(Path path)` is called with Path to empty directory
- **THEN** directory is removed

#### Scenario: Directory not empty
- **WHEN** `fun_filesystem_remove_directory(Path path)` is called with Path to a non-empty directory
- **THEN** an error with code `ERROR_CODE_DIRECTORY_NOT_EMPTY` is returned

#### Scenario: Directory not found
- **WHEN** `fun_filesystem_remove_directory(Path path)` is called with a Path that does not exist
- **THEN** an error with code `ERROR_CODE_DIRECTORY_NOT_FOUND` is returned

#### Scenario: Inaccessible directory treated as non-empty
- **WHEN** `fun_filesystem_remove_directory(Path path)` is called and the directory cannot be opened to check emptiness
- **THEN** the operation SHALL NOT proceed and an error is returned
- **AND** the directory is NOT removed

## ADDED Requirements

### Requirement: Path Serialisation Length Limit
The filesystem module SHALL reject path serialisation when the path string representation exceeds the internal buffer size.

#### Scenario: Path too long for internal buffer
- **WHEN** a filesystem operation (create, remove, list, exists, size) is called with a Path whose string representation exceeds the internal 512-byte serialisation buffer
- **THEN** the operation returns an error with code `ERROR_CODE_PATH_TOO_LONG`
- **AND** no filesystem state is modified

#### Scenario: Path within buffer limit succeeds
- **WHEN** a filesystem operation is called with a Path whose string representation fits within the 512-byte buffer
- **THEN** the operation proceeds normally
