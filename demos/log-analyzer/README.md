# Log Analyzer Demo

A simple log file analyzer that demonstrates Fundamental Library's file I/O, string operations, and collections.

## Features

- Reads log files asynchronously using memory-mapped I/O
- Parses log entries in format: `[TIMESTAMP] [LEVEL] message`
- Counts occurrences of each log level (INFO, WARN, ERROR, DEBUG, TRACE)
- Displays summary statistics

## Build

### Windows
```batch
.\build-windows-amd64.bat
```

### Linux
```bash
./build-linux-amd64.sh
```

## Usage

```batch
demo.exe <logfile>
```

### Example

Create a test log file:
```
[2024-01-15 10:30:45] [INFO] Application started
[2024-01-15 10:30:46] [DEBUG] Loading configuration
[2024-01-15 10:30:47] [ERROR] Database connection failed
[2024-01-15 10:30:48] [WARN] Retrying connection
[2024-01-15 10:30:49] [INFO] Connected successfully
```

Run the analyzer:
```batch
demo.exe test.log
```

Expected output:
```
=== Log Analyzer Demo ===
Analyzing: test.log

=== Results ===
INFO: 2
WARN: 1
ERROR: 1
DEBUG: 1
TRACE: 0

Total lines: 5
```

## Dependencies

- File I/O (async read)
- String operations
- Console I/O
- Hashmap (for counting)
- Memory management
- Async operations

## Notes

- Maximum file size: 64KB (buffer size in demo)
- Log format must match: `[TIMESTAMP] [LEVEL] message`
- Supported levels: INFO, WARN, ERROR, DEBUG, TRACE
