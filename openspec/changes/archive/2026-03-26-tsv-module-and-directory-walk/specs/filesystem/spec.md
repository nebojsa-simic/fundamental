## MODIFIED Requirements

### Requirement: Directory List Operation
The filesystem module SHALL provide function to list directory contents using Path type, with each entry prefixed by its type.

#### Scenario: List directory contents
- **WHEN** `fun_filesystem_list_directory(Path path, Memory output)` is called
- **THEN** directory entries are written to output buffer in TSV format: `D\tname\n` for directories and `F\tname\n` for regular files
- **AND** entries are separated by newlines
- **AND** Path is converted to native string for system call

#### Scenario: Entry type detection at listing time
- **WHEN** `fun_filesystem_list_directory` is called
- **THEN** each entry's type is determined using information available from the OS directory iteration (no additional stat calls per entry)
