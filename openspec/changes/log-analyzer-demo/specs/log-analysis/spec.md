## ADDED Requirements

### Requirement: Log file parsing
The system SHALL read log files asynchronously and parse individual log entries to extract structured information including timestamp, log level, and message content.

#### Scenario: Successful file read
- **WHEN** a valid log file path is provided
- **THEN** the system SHALL read the entire file into memory using async I/O

#### Scenario: Line-by-line parsing
- **WHEN** the file content is loaded
- **THEN** the system SHALL parse each line to extract timestamp, level, and message components

#### Scenario: Log format recognition
- **WHEN** a line matches the format `[TIMESTAMP] [LEVEL] message`
- **THEN** the system SHALL correctly extract all three components

### Requirement: Level counting
The system SHALL count occurrences of each log level (TRACE, DEBUG, INFO, WARN, ERROR) and display summary statistics.

#### Scenario: Count by level
- **WHEN** log entries are parsed
- **THEN** the system SHALL increment counters for each encountered log level

#### Scenario: Handle unknown levels
- **WHEN** a log entry has an unrecognized level
- **THEN** the system SHALL count it as "UNKNOWN" and continue processing

#### Scenario: Display summary
- **WHEN** all entries are processed
- **THEN** the system SHALL display counts for each level in a formatted table

### Requirement: Time-based statistics
The system SHALL aggregate log entries by time periods (hour and minute) to show temporal distribution.

#### Scenario: Extract timestamp
- **WHEN** parsing a log entry
- **THEN** the system SHALL extract the timestamp portion and parse hour/minute components

#### Scenario: Group by hour
- **WHEN** entries are processed
- **THEN** the system SHALL aggregate counts by hour (00-23)

#### Scenario: Group by minute
- **WHEN** finer granularity is requested
- **THEN** the system SHALL aggregate counts by minute within each hour

#### Scenario: Display time distribution
- **WHEN** showing time statistics
- **THEN** the system SHALL display entries per hour in chronological order

### Requirement: Pattern filtering
The system SHALL filter log entries based on substring pattern matching to show only relevant entries.

#### Scenario: Apply pattern filter
- **WHEN** a pattern is provided via command-line flag
- **THEN** the system SHALL only display entries containing the pattern in the message

#### Scenario: Case-sensitive matching
- **WHEN** pattern matching is performed
- **THEN** the system SHALL perform case-sensitive substring matching

#### Scenario: No matches found
- **WHEN** no entries match the filter pattern
- **THEN** the system SHALL display a message indicating zero matches

### Requirement: Summary output
The system SHALL generate a formatted summary report to the console with optional file output capability.

#### Scenario: Console output
- **WHEN** analysis completes
- **THEN** the system SHALL display a formatted summary including total entries, level counts, and time distribution

#### Scenario: File output (optional)
- **WHEN** an output file path is specified
- **THEN** the system SHALL write the detailed analysis results to the specified file

#### Scenario: Error reporting
- **WHEN** errors occur during processing
- **THEN** the system SHALL display clear error messages with appropriate error codes
