## REMOVED Requirements

### Requirement: Platform-specific path separator
**Reason**: Path type eliminates need for separator character - components are stored as array without separators

**Migration**: Remove all calls to `fun_path_separator()`. Path operations now work with component arrays, not separator-delimited strings. Use `fun_path_from_string()` to parse strings and `fun_path_to_string()` to generate platform-specific strings at I/O boundaries.

## MODIFIED Requirements

### Requirement: Path Join Operation
The filesystem module SHALL provide function to join path components.

#### Scenario: Join two path components
- **WHEN** `fun_path_join(Path base, Path relative, Path output)` is called with valid Paths
- **THEN** output Path contains components from base followed by components from relative
- **AND** dot-dot (..) and dot (.) components are resolved during join
- **AND** output.is_absolute = base.is_absolute

### Requirement: Path Normalize Operation
The filesystem module SHALL provide function to normalize a Path by resolving relative components.

#### Scenario: Normalize path with relative components
- **WHEN** `fun_path_normalize(Path path, Path output)` is called
- **THEN** all `.` components are removed
- **AND** all `..` components resolve by removing preceding component
- **AND** `..` at root are safely discarded (cannot escape root)

### Requirement: Path Parent Extraction
The filesystem module SHALL provide function to extract parent directory from Path.

#### Scenario: Get parent of multi-component path
- **WHEN** `fun_path_get_parent(Path path, Path output)` is called
- **THEN** output contains all components except the last
- **AND** output.is_absolute = path.is_absolute

### Requirement: Path Filename Extraction
The filesystem module SHALL provide function to extract filename from Path.

#### Scenario: Get filename from path
- **WHEN** `fun_path_get_filename(Path path, Path output)` is called
- **THEN** output contains only the last component
- **AND** output.count = 1

### Requirement: Directory Create Operation
The filesystem module SHALL provide function to create directories using Path type.

#### Scenario: Create directory with Path
- **WHEN** `fun_filesystem_create_directory(Path path)` is called with valid Path
- **THEN** directory is created at specified location
- **AND** parent directories are created if they don't exist
- **AND** Path is converted to native string for system call

### Requirement: Directory Remove Operation
The filesystem module SHALL provide function to remove directories using Path type.

#### Scenario: Remove empty directory
- **WHEN** `fun_filesystem_remove_directory(Path path)` is called with Path to empty directory
- **THEN** directory is removed
- **AND** error is returned if directory is not empty

### Requirement: Directory List Operation
The filesystem module SHALL provide function to list directory contents using Path type.

#### Scenario: List directory contents
- **WHEN** `fun_filesystem_list_directory(Path path, Memory output)` is called
- **THEN** directory entry names are written to output buffer
- **AND** entries are separated by newlines
- **AND** Path is converted to native string for system call
