## ADDED Requirements

### Requirement: Console Output Skill Structure
The AI agent skill for console output SHALL provide copy-paste examples for console operations.

#### Scenario: Skill file location
- **WHEN** an AI agent needs console output functionality
- **THEN** the agent can find the skill at `.opencode/skills/fundamental-console.md`

### Requirement: Basic Output Examples
The console skill SHALL demonstrate basic text output to console.

#### Scenario: Write text example
- **WHEN** the skill shows console output
- **THEN** it uses `fun_console_write("text")` for output without newline
- **AND** it explains buffering behavior

#### Scenario: Write line example
- **WHEN** the skill shows line output
- **THEN** it uses `fun_console_write_line("text")` for output with newline
- **AND** it shows when to use write vs write_line

### Requirement: Progress Bar Example
The console skill SHALL demonstrate rendering a progress bar.

#### Scenario: Progress bar shows initialization
- **WHEN** the skill demonstrates progress bar
- **THEN** it shows setting up progress state (total, current)
- **AND** it allocates any needed buffers

#### Scenario: Progress bar shows update pattern
- **WHEN** the skill demonstrates progress updates
- **THEN** it shows updating progress in a loop
- **AND** it demonstrates clearing/redrawing for animation effect

#### Scenario: Progress bar shows completion
- **WHEN** the skill demonstrates progress completion
- **THEN** it shows final newline after progress completes
- **AND** it cleans up any allocated resources

### Requirement: Error Output Examples
The console skill SHALL demonstrate error message output.

#### Scenario: Error line example
- **WHEN** the skill shows error output
- **THEN** it uses `fun_console_error_line("message")` for errors
- **AND** it explains error output goes to stderr

### Requirement: Console Flush Example
The console skill SHALL demonstrate flushing console output buffer.

#### Scenario: Flush example shows explicit flush
- **WHEN** the skill shows output flushing
- **THEN** it uses `fun_console_flush()` to force output
- **AND** it explains when flush is necessary (before long operations, progress bars)
