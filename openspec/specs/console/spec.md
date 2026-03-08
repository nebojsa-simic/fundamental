# Console Module Specification

## Purpose
Console module provides line-buffered output to stdout and stderr streams without stdlib runtime dependencies, enabling CLI applications to write formatted output using fundamental library exclusively.

## Requirements

### Requirement: Console Write Line
The system SHALL provide function to write a null-terminated string to stdout with automatic newline.

#### Scenario: Write string to stdout with newline
- **WHEN** fun_console_write_line is called with valid string
- **THEN** string is written to stdout followed by newline character
- **AND** output is line-buffered (flushed on newline or buffer full)

#### Scenario: Write empty string to stdout
- **WHEN** fun_console_write_line is called with empty string
- **THEN** only newline character is written to stdout

#### Scenario: Write very long line
- **WHEN** fun_console_write_line is called with string exceeding buffer size
- **THEN** buffer is flushed and string is written directly
- **AND** newline is appended at end

### Requirement: Console Error Line
The system SHALL provide function to write a null-terminated string to stderr with automatic newline.

#### Scenario: Write error message to stderr
- **WHEN** fun_console_error_line is called with valid string
- **THEN** string is written to stderr followed by newline character
- **AND** stderr output is not buffered (immediate flush)

#### Scenario: Write error after stdout
- **WHEN** fun_console_error_line is called after fun_console_write_line
- **THEN** error message appears on stderr stream separately from stdout
- **AND** stream separation is maintained

### Requirement: Console Write (Raw)
The system SHALL provide function to write a null-terminated string to stdout without automatic newline.

#### Scenario: Write string without newline
- **WHEN** fun_console_write is called with valid string
- **THEN** string is written to stdout buffer without newline
- **AND** buffer is not flushed (awaits more data or explicit flush)

#### Scenario: Build output incrementally
- **WHEN** fun_console_write is called multiple times followed by fun_console_write_line
- **THEN** all output is concatenated and flushed together
- **AND** single newline is appended at end

### Requirement: Console Flush
The system SHALL provide function to explicitly flush console output buffer.

#### Scenario: Flush buffer explicitly
- **WHEN** fun_console_flush is called
- **THEN** all buffered output is written to stdout immediately
- **AND** buffer position is reset to zero

#### Scenario: Flush empty buffer
- **WHEN** fun_console_flush is called with empty buffer
- **THEN** function returns successfully with no output
- **AND** no error is reported

### Requirement: Platform Abstraction
The system SHALL abstract platform-specific console APIs transparently.

#### Scenario: Windows console output
- **WHEN** console functions are called on Windows
- **THEN** WriteFile() with GetStdHandle() is used internally
- **AND** caller sees consistent behavior with POSIX

#### Scenario: POSIX console output
- **WHEN** console functions are called on POSIX systems
- **THEN** write() syscall is used internally
- **AND** caller sees consistent behavior with Windows

### Requirement: Zero Stdlib Runtime
The system SHALL not use stdlib runtime functions in implementation.

#### Scenario: No printf family usage
- **WHEN** console module is compiled
- **THEN** no printf, fprintf, puts, putchar calls are present
- **AND** only platform APIs (WriteFile, write) are used

#### Scenario: No malloc family usage
- **WHEN** console module buffers are allocated
- **THEN** static allocation is used, not malloc/realloc
- **AND** no dynamic memory management in console module

## Constraints
- Buffer size SHALL be 512 bytes (compile-time constant)
- All functions MUST validate string parameter is not NULL
- Stderr output MUST NOT be buffered (immediate flush)
- Functions MUST NOT use stdlib runtime functions (printf, malloc, etc.)
- Thread safety is NOT guaranteed (single-threaded usage assumed)
