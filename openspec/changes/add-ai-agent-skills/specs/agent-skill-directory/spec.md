## ADDED Requirements

### Requirement: Directory Operations Skill Structure
The AI agent skill for directory operations SHALL provide copy-paste examples for directory tasks.

#### Scenario: Skill file location
- **WHEN** an AI agent needs directory operations
- **THEN** the agent can find the skill at `.opencode/skills/fundamental-directory.md`

### Requirement: Directory Create Example
The directory skill SHALL demonstrate creating directories.

#### Scenario: Create directory includes error handling
- **WHEN** the skill shows directory creation
- **THEN** it uses `fun_filesystem_create_directory(path)` with ErrorResult checking
- **AND** it handles DIRECTORY_EXISTS error gracefully

### Requirement: Directory List Example
The directory skill SHALL demonstrate listing directory contents.

#### Scenario: List directory shows buffer allocation
- **WHEN** the skill demonstrates directory listing
- **THEN** it allocates buffer for output with `fun_memory_allocate()`
- **AND** it specifies appropriate buffer size (4096 or larger)

#### Scenario: List directory shows parsing
- **WHEN** the skill demonstrates parsing directory listing
- **THEN** it shows splitting newline-separated entries
- **AND** it iterates over entries safely

### Requirement: Directory Remove Example
The directory skill SHALL demonstrate removing directories.

#### Scenario: Remove directory shows preconditions
- **WHEN** the skill demonstrates directory removal
- **THEN** it explains directory must be empty
- **AND** it shows checking emptiness or recursive remove pattern

### Requirement: Directory Existence Check Example
The directory skill SHALL demonstrate checking if a directory exists.

#### Scenario: Directory exists example uses correct API
- **WHEN** the skill demonstrates directory existence checking
- **THEN** it uses `fun_directory_exists()` function (when implemented)
- **AND** it handles the boolResult return type correctly

### Requirement: Directory Iteration Example
The directory skill SHALL demonstrate iterating over files in a directory.

#### Scenario: Iteration shows complete pattern
- **WHEN** the skill demonstrates directory iteration
- **THEN** it shows: list → parse → iterate → process each file
- **AND** it includes error handling for each step

#### Scenario: Iteration includes cleanup
- **WHEN** the skill demonstrates directory iteration
- **THEN** it frees listing buffer after iteration completes
- **AND** it cleans up on error paths too
