## ADDED Requirements

### Requirement: Path Separator Access
The filesystem module SHALL retain the `fun_path_separator()` function to provide platform-specific path separator character for manual string construction scenarios.

#### Scenario: Get separator on Windows
- **WHEN** `fun_path_separator()` is called on Windows platform
- **THEN** the function returns backslash character `'\\'`

#### Scenario: Get separator on POSIX
- **WHEN** `fun_path_separator()` is called on POSIX/Linux platform
- **THEN** the function returns forward slash character `'/'`

#### Scenario: Manual path construction use case
- **WHEN** user needs to manually construct or parse path strings outside the Path API
- **THEN** `fun_path_separator()` provides the correct character for the current platform

### Requirement: Path Type Definition
The filesystem module SHALL define a Path type that represents file system paths as arrays of string components without platform-specific separators.

#### Scenario: Path struct definition
- **WHEN** the Path type is defined
- **THEN** it contains a `components` field as array of `const char *` pointers
- **AND** it contains a `count` field indicating number of components
- **AND** it contains an `is_absolute` boolean field indicating absolute vs relative path

#### Scenario: OutputPath typedef definition
- **WHEN** the OutputPath type is defined
- **THEN** it is a typedef for `Path *` to follow library output parameter conventions
- **AND** it is used for all functions that output Path values

#### Scenario: Path component representation
- **WHEN** a Path represents a directory structure
- **THEN** each component is a single directory or filename without separators
- **AND** components do not contain `/`, `\`, `.`, or `..` as values (these are structural, not components)

### Requirement: String to Path Conversion
The filesystem module SHALL provide functions to convert null-terminated strings to Path structures.

#### Scenario: Parse absolute Unix path
- **WHEN** `fun_path_from_string(String path, OutputPath output)` is called with path "/home/user/documents/file.txt"
- **THEN** the resulting Path has `is_absolute = true`
- **AND** components = ["home", "user", "documents", "file.txt"]
- **AND** count = 4

#### Scenario: Parse relative path
- **WHEN** `fun_path_from_string(String path, OutputPath output)` is called with path "documents/file.txt"
- **THEN** the resulting Path has `is_absolute = false`
- **AND** components = ["documents", "file.txt"]
- **AND** count = 2

#### Scenario: Parse path with dot components
- **WHEN** `fun_path_from_string(String path, OutputPath output)` is called with path "./documents/../file.txt"
- **THEN** the resulting Path has components = [".", "documents", "..", "file.txt"] (preserved, not normalized)
- **AND** normalization requires separate call to `fun_path_normalize(Path path, OutputPath output)`

#### Scenario: Parse path with multiple separators
- **WHEN** `fun_path_from_string(String path, OutputPath output)` encounters `//` or `///` in path
- **THEN** multiple consecutive separators are treated as single separator
- **AND** no empty components are created

#### Scenario: Parse trailing separator
- **WHEN** `fun_path_from_string(String path, OutputPath output)` is called with trailing separator
- **THEN** trailing separator is ignored
- **AND** components = ["home", "user"] (not ["home", "user", ""])

#### Scenario: Parse Windows absolute path
- **WHEN** `fun_path_from_string(String path, OutputPath output)` is called with path "C:/Users/Documents/file.txt" on Windows
- **THEN** the resulting Path has `is_absolute = true`
- **AND** first component may include drive letter or drive letter is stored separately
- **AND** remaining components = ["Users", "Documents", "file.txt"]

#### Scenario: Handle invalid input
- **WHEN** `fun_path_from_string(String path, OutputPath output)` is called with NULL pointer
- **THEN** error is returned with `ERROR_CODE_NULL_POINTER`

#### Scenario: Handle empty string
- **WHEN** `fun_path_from_string(String path, OutputPath output)` is called with empty string
- **THEN** the resulting Path has count = 0
- **AND** `is_absolute = false`

### Requirement: Path to String Conversion
The filesystem module SHALL provide functions to convert Path structures back to null-terminated strings with platform-appropriate separators.

#### Scenario: Convert to Unix-style string
- **WHEN** `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` is called on POSIX platform
- **THEN** components are joined with `/` separator
- **AND** absolute paths start with `/`

#### Scenario: Convert to Windows-style string
- **WHEN** `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` is called on Windows platform
- **THEN** components are joined with `\` separator
- **AND** absolute paths include drive letter if present

#### Scenario: Convert relative path
- **WHEN** `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` is called with relative Path
- **THEN** output string does not start with separator
- **AND** components are joined with platform separator

#### Scenario: Handle empty Path
- **WHEN** `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` is called with Path where count = 0
- **THEN** output is empty string or current directory indicator based on platform

#### Scenario: Buffer size validation
- **WHEN** `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` output buffer is too small
- **THEN** error is returned with `ERROR_CODE_BUFFER_TOO_SMALL`
- **AND** no partial data is written to buffer

### Requirement: Path Join Operation
The filesystem module SHALL provide function to join two Paths into a single Path.

#### Scenario: Join base and relative paths
- **WHEN** `fun_path_join(Path base, Path relative, OutputPath output)` is called
- **THEN** output contains all components from base followed by components from relative
- **AND** output.is_absolute = base.is_absolute

#### Scenario: Join with dot-dot component
- **WHEN** relative path contains `..` component
- **THEN** `..` removes the last component from base (if any exists)
- **AND** if base is empty, `..` is preserved in output

#### Scenario: Join with dot component
- **WHEN** relative path contains `.` component
- **THEN** `.` is ignored (current directory, no effect on output)

#### Scenario: Join empty relative path
- **WHEN** relative Path has count = 0
- **THEN** output equals base (copy of base components)

#### Scenario: Join empty base path
- **WHEN** base Path has count = 0
- **THEN** output equals relative (copy of relative components)

### Requirement: Path Normalize Operation
The filesystem module SHALL provide function to normalize a Path by resolving `.` and `..` components.

#### Scenario: Normalize path with dot components
- **WHEN** `fun_path_normalize(Path path, OutputPath output)` is called with `.` components
- **THEN** all `.` components are removed from output

#### Scenario: Normalize path with dot-dot components
- **WHEN** `fun_path_normalize(Path path, OutputPath output)` is called with `..` components
- **THEN** each `..` removes preceding component (if exists)
- **AND** `..` that would go before root are removed (cannot escape root)

#### Scenario: Normalize already clean path
- **WHEN** `fun_path_normalize(Path path, OutputPath output)` is called with path having no `.` or `..`
- **THEN** output is identical to input

#### Scenario: Normalize empty path
- **WHEN** `fun_path_normalize(Path path, OutputPath output)` is called with Path where count = 0
- **THEN** output is empty path (no error)

### Requirement: Path Parent Extraction
The filesystem module SHALL provide function to extract parent directory from a Path.

#### Scenario: Get parent of nested path
- **WHEN** `fun_path_get_parent(Path path, OutputPath output)` is called with Path having multiple components
- **THEN** output contains all components except the last
- **AND** output.count = path.count - 1

#### Scenario: Get parent of single component
- **WHEN** `fun_path_get_parent(Path path, OutputPath output)` is called with Path having one component
- **THEN** output has count = 0 (empty path)

#### Scenario: Get parent of root path
- **WHEN** `fun_path_get_parent(Path path, OutputPath output)` is called with absolute Path having one component
- **THEN** output is absolute path with count = 0 (represents root)

#### Scenario: Get parent of empty path
- **WHEN** `fun_path_get_parent(Path path, OutputPath output)` is called with Path where count = 0
- **THEN** error is returned or empty path (no parent exists)

### Requirement: Path Filename Extraction
The filesystem module SHALL provide function to extract filename (last component) from a Path.

#### Scenario: Get filename from path
- **WHEN** `fun_path_get_filename(Path path, OutputPath output)` is called with Path having at least one component
- **THEN** output contains only the last component
- **AND** output.count = 1

#### Scenario: Get filename from single component
- **WHEN** `fun_path_get_filename(Path path, OutputPath output)` is called with Path having one component
- **THEN** output equals input (single component is the filename)

#### Scenario: Get filename from empty path
- **WHEN** `fun_path_get_filename(Path path, OutputPath output)` is called with Path where count = 0
- **THEN** error is returned or empty string (no filename exists)

### Requirement: Path Component Access
The filesystem module SHALL provide functions to access individual Path components.

#### Scenario: Get component by index
- **WHEN** `fun_path_get_component(Path path, size_t index)` is called with valid index
- **THEN** the component string at that index is returned
- **AND** 0 <= index < path.count

#### Scenario: Access invalid component index
- **WHEN** `fun_path_get_component(Path path, size_t index)` is called with index >= path.count
- **THEN** NULL is returned or error code indicates out of bounds

#### Scenario: Get component count
- **WHEN** `fun_path_component_count(Path path)` is called
- **THEN** the number of components is returned as size_t

### Requirement: Path Validation
The filesystem module SHALL provide function to validate Path structure.

#### Scenario: Validate well-formed Path
- **WHEN** `fun_path_is_valid(Path path)` is called with properly constructed Path
- **THEN** true is returned
- **AND** all components are non-NULL
- **AND** all components are non-empty strings

#### Scenario: Validate Path with NULL component
- **WHEN** `fun_path_is_valid(Path path)` is called with Path containing NULL in components array
- **THEN** false is returned

#### Scenario: Validate Path with empty component
- **WHEN** `fun_path_is_valid(Path path)` is called with Path containing empty string component
- **THEN** false is returned

## MODIFIED Requirements

### Requirement: Filesystem Module Path Parameters
The filesystem module SHALL accept Path type instead of String for all path parameters.

#### Scenario: Create directory with Path
- **WHEN** `fun_filesystem_create_directory(Path path)` is called
- **THEN** the directory is created at the location specified by Path
- **AND** Path is converted to native string for system call

#### Scenario: Remove directory with Path
- **WHEN** `fun_filesystem_remove_directory(Path path)` is called
- **THEN** the directory is removed if empty
- **AND** Path is converted to native string for system call

#### Scenario: List directory with Path
- **WHEN** `fun_filesystem_list_directory(Path path, Memory output)` is called
- **THEN** directory contents are listed
- **AND** Path is converted to native string for system call
