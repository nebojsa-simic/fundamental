## Why

Developers and system administrators frequently need to analyze log files to identify issues, track application behavior, and generate reports. Currently, the Fundamental Library demos don't showcase file I/O, string parsing, and data aggregation capabilities in a practical, real-world application. A log analyzer demonstrates these core features while providing genuine utility.

## What Changes

- **New demo application** in `demos/log-analyzer/` with complete build scripts for Windows and Linux
- **Log parsing functionality** that reads files asynchronously and extracts structured information
- **Statistics generation** including counts by log level, time-based analysis, and pattern matching
- **Console output** with formatted summary reports
- **Optional file output** for saving analysis results

## Capabilities

### New Capabilities

- `log-analysis`: Parse log files, extract entries by level, generate time-based statistics, filter by patterns, and output summaries
- `file-processing`: Demonstrate async file I/O with line-by-line processing and error handling

### Modified Capabilities

<!-- No existing capabilities are being modified - this is a new demo application -->

## Impact

- **New demo directory**: `demos/log-analyzer/` with demo.c and build scripts
- **Dependencies**: File I/O module (async read), String module (parsing, templates), Console module (output), Collections (hashmap for counting), Array (storing entries)
- **No breaking changes**: This is additive functionality that doesn't affect existing code
- **Platform support**: Windows AMD64 and Linux AMD64 (following existing demo patterns)
