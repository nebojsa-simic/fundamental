## ADDED Requirements

### Requirement: File I/O Skill Structure
The AI agent skill for file I/O SHALL provide copy-paste examples for common file operations.

#### Scenario: Skill file location
- **WHEN** an AI agent needs to perform file I/O operations
- **THEN** the agent can find the skill at `.opencode/skills/fundamental-file-io.md`

#### Scenario: Skill frontmatter
- **WHEN** the skill file is parsed
- **THEN** it contains YAML frontmatter with name, description, license, and metadata fields

### Requirement: File Read Example
The file I/O skill SHALL demonstrate reading a file from disk with proper error handling.

#### Scenario: Read file example includes allocation
- **WHEN** the skill shows reading a file
- **THEN** it includes `fun_memory_allocate()` call with error checking
- **AND** it shows buffer size selection rationale

#### Scenario: Read file example uses correct API
- **WHEN** the skill demonstrates file reading
- **THEN** it uses `fun_read_file_in_memory()` with proper struct initialization
- **AND** it calls `fun_async_await()` to wait for completion

#### Scenario: Read file example includes cleanup
- **WHEN** the skill shows file reading
- **THEN** it includes `fun_memory_free()` call at the end
- **AND** cleanup happens even if errors occur

### Requirement: File Write Example
The file I/O skill SHALL demonstrate writing data to a file.

#### Scenario: Write file example shows buffer preparation
- **WHEN** the skill demonstrates file writing
- **THEN** it shows preparing data in a buffer before writing
- **AND** it specifies bytes_to_write parameter correctly

#### Scenario: Write file example uses correct API
- **WHEN** the skill demonstrates file writing
- **THEN** it uses `fun_write_memory_to_file()` with proper parameters
- **AND** it handles async result properly

### Requirement: File Existence Check Example
The file I/O skill SHALL demonstrate checking if a file exists.

#### Scenario: File exists example uses correct API
- **WHEN** the skill demonstrates existence checking
- **THEN** it uses `fun_file_exists()` function (when implemented)
- **AND** it handles the boolResult return type correctly

### Requirement: File Append Example
The file I/O skill SHALL demonstrate appending data to an existing file.

#### Scenario: Append file example shows correct API usage
- **WHEN** the skill demonstrates file appending
- **THEN** it uses `fun_append_memory_to_file()` function
- **AND** it explains difference between append and write modes

### Requirement: Error Handling Patterns
The file I/O skill SHALL demonstrate comprehensive error handling for all file operations.

#### Scenario: Error checking after allocation
- **WHEN** the skill shows any file operation
- **THEN** it checks `fun_error_is_error(result.error)` after memory allocation
- **AND** it returns or handles error appropriately

#### Scenario: Error checking after file operation
- **WHEN** the skill shows file read/write operations
- **THEN** it checks async result status for ASYNC_ERROR
- **AND** it explains common error codes (NOT_FOUND, PERMISSION_DENIED)

### Requirement: Memory Management Examples
The file I/O skill SHALL demonstrate proper memory lifecycle for file operations.

#### Scenario: Memory allocation pattern
- **WHEN** the skill shows buffer allocation
- **THEN** it follows pattern: allocate → check error → use → free
- **AND** it explains caller-allocated memory responsibility

#### Scenario: Memory cleanup on error paths
- **WHEN** the skill shows error handling
- **THEN** it frees allocated memory before returning on error
- **AND** it prevents memory leaks in all code paths
