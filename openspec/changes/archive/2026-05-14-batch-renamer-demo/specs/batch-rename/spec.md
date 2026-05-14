## ADDED Requirements

### Requirement: Batch file renaming
The system SHALL rename multiple files in a directory based on user-specified transformation rules with preview capability.

#### Scenario: Single file rename
- **WHEN** a single file is selected for renaming
- **THEN** the system SHALL rename the file to the new name without errors

#### Scenario: Multiple file rename
- **WHEN** multiple files match the selection criteria
- **THEN** the system SHALL rename all matching files in a single operation

#### Scenario: Invalid source path
- **WHEN** the specified directory does not exist
- **THEN** the system SHALL display an error message and exit gracefully

### Requirement: Find and replace mode
The system SHALL support renaming files by finding a substring in the filename and replacing it with another string.

#### Scenario: Simple replacement
- **WHEN** user specifies `--find-replace "old" "new"`
- **THEN** the system SHALL replace all occurrences of "old" with "new" in matching filenames

#### Scenario: No matches
- **WHEN** the find pattern does not match any part of the filename
- **THEN** the system SHALL leave the filename unchanged and continue

#### Scenario: Empty replacement
- **WHEN** the replacement string is empty
- **THEN** the system SHALL remove all occurrences of the find pattern from the filename

### Requirement: Prefix and suffix mode
The system SHALL support adding text before (prefix) or after (suffix) the base filename while preserving the extension.

#### Scenario: Add prefix only
- **WHEN** user specifies `--prefix "pre_"`
- **THEN** the system SHALL add "pre_" to the beginning of each filename (before the base name)

#### Scenario: Add suffix only
- **WHEN** user specifies `--suffix "_suf"`
- **THEN** the system SHALL add "_suf" to the end of each filename (after the base name, before extension)

#### Scenario: Add both prefix and suffix
- **WHEN** user specifies both `--prefix` and `--suffix`
- **THEN** the system SHALL apply both transformations in the correct order

### Requirement: Sequential numbering mode
The system SHALL add sequential numbers to filenames with zero-padding to maintain sort order.

#### Scenario: Number files sequentially
- **WHEN** user specifies `--number`
- **THEN** the system SHALL add numbers (001, 002, 003, etc.) to each filename

#### Scenario: Zero-padding
- **WHEN** numbering is applied
- **THEN** the system SHALL use 3-digit zero-padded numbers by default

#### Scenario: Preserve extension
- **WHEN** numbering is applied to files with extensions
- **THEN** the system SHALL insert the number before the file extension

### Requirement: Extension change mode
The system SHALL support changing file extensions in bulk for all matching files.

#### Scenario: Change extension
- **WHEN** user specifies `--extension ".new"`
- **THEN** the system SHALL replace the existing extension with ".new" for all matching files

#### Scenario: Remove extension
- **WHEN** user specifies `--extension ""` (empty string)
- **THEN** the system SHALL remove the extension from all matching files

#### Scenario: Add extension to files without one
- **WHEN** a file has no extension and user specifies `--extension ".txt"`
- **THEN** the system SHALL add the extension to the filename

### Requirement: Wildcard pattern matching
The system SHALL support wildcard patterns (* and ?) for selecting files to rename.

#### Scenario: Asterisk wildcard
- **WHEN** user specifies pattern `*.txt`
- **THEN** the system SHALL match all files with .txt extension

#### Scenario: Question mark wildcard
- **WHEN** user specifies pattern `file?.txt`
- **THEN** the system SHALL match file1.txt, file2.txt, etc. (single character match)

#### Scenario: No wildcard
- **WHEN** user specifies exact filename without wildcards
- **THEN** the system SHALL match only that exact filename

### Requirement: Dry-run preview
The system SHALL default to dry-run mode that previews changes without actually renaming files.

#### Scenario: Default dry-run
- **WHEN** user runs the command without `--apply` flag
- **THEN** the system SHALL display what would be renamed without making changes

#### Scenario: Show before/after
- **WHEN** in dry-run mode
- **THEN** the system SHALL display both original and new filenames for each match

#### Scenario: Apply changes
- **WHEN** user specifies `--apply` flag
- **THEN** the system SHALL perform the actual file renaming operations

### Requirement: Conflict detection
The system SHALL detect and report potential filename conflicts before applying changes.

#### Scenario: Target file exists
- **WHEN** a new filename already exists in the directory
- **THEN** the system SHALL report this as a conflict in dry-run mode

#### Scenario: Skip conflicts in apply mode
- **WHEN** conflicts exist and user specifies `--apply`
- **THEN** the system SHALL skip conflicting files and report them as errors

#### Scenario: Self-reference
- **WHEN** a rename operation would result in the same filename
- **THEN** the system SHALL skip the file and continue without error
