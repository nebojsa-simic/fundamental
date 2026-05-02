## Context

The Fundamental Library provides all necessary building blocks (file I/O, string operations, collections, console I/O) but lacks demonstration applications that combine multiple modules in practical, real-world scenarios. This demo will showcase async file operations, string parsing, data aggregation with hashmaps, and formatted console output.

**Constraints:**
- Must follow existing demo patterns (build scripts for Windows/Linux)
- Must use only Fundamental Library APIs (no stdlib dependencies in logic)
- Must handle errors gracefully with proper error checking
- Code should be minimal but complete - demonstrate features without bloat

## Goals / Non-Goals

**Goals:**
- Parse log files line-by-line using async file I/O
- Extract log levels (INFO, WARN, ERROR, DEBUG, TRACE) and count occurrences
- Extract timestamps and aggregate by hour/minute
- Support pattern filtering to show only matching entries
- Generate formatted summary report to console
- Optionally write detailed results to output file
- Demonstrate hashmap for counting, arrays for storage, string parsing

**Non-Goals:**
- Real-time log tailing or streaming analysis
- Multi-threaded or parallel processing
- Support for complex log formats (JSON, XML)
- GUI or interactive mode
- Log rotation or file management

## Decisions

### 1. File Reading Strategy
**Decision:** Use async file read with memory-mapped I/O for entire file, then parse line-by-line in memory.

**Rationale:** 
- Simpler than line-by-line async reads for a demo
- Memory-mapped I/O is efficient for files up to hundreds of MB
- Allows random access for multiple passes (counting, filtering, output)
- Follows the pattern from existing fileRead tests

**Alternatives considered:**
- Line-by-line async reads: More complex, better for huge files (>1GB)
- Stream-based I/O: Overkill for this use case, adds complexity

### 2. Log Format Assumption
**Decision:** Support a simple, common log format: `[TIMESTAMP] [LEVEL] message`

**Example:** `[2024-01-15 10:30:45] [ERROR] Database connection failed`

**Rationale:**
- Common format used by many applications
- Easy to parse with string operations
- Demonstrates substring extraction and comparison
- Can be extended for other formats

**Alternatives considered:**
- Apache/nginx combined log format: Too complex for demo
- JSON logs: Requires JSON parser (not in Fundamental Library yet)
- Syslog format: Too many variations

### 3. Data Structures
**Decision:** Use HashMap for level counts, Array for storing filtered entries

**Rationale:**
- HashMap provides O(1) lookup for incrementing level counts
- Array provides ordered storage for entries matching filter
- Demonstrates both collection types effectively
- Type-safe macros from Fundamental Library

### 4. Pattern Matching
**Decision:** Simple substring match using `fun_string_index_of()`

**Rationale:**
- No regex engine in Fundamental Library
- Substring matching covers 80% of use cases
- Fast and simple to implement
- Demonstrates string search functionality

**Alternatives considered:**
- Full regex: Would require external library
- Glob patterns: More complex, less useful for log messages
- Exact match only: Too limiting

### 5. Output Format
**Decision:** Use console output with string templates for formatting

**Rationale:**
- Demonstrates `fun_console_write_line()` and templates
- Immediate feedback for user
- Can redirect to file via shell if needed
- Optional file output for detailed reports

## Risks / Trade-offs

**[Risk] Large files (>1GB) may consume excessive memory**
→ Mitigation: Document limitation, mention stream-based approach for production use

**[Risk] Log format variations may not parse correctly**
→ Mitigation: Clearly document expected format, add error handling for malformed lines

**[Risk] Timezone handling complexity**
→ Mitigation: Treat timestamps as strings for grouping, avoid parsing timezone info

**[Trade-off] Simplicity vs. flexibility**
→ Decision: Favor simplicity for demo purposes. Production version could add config for formats

**[Trade-off] Performance vs. readability**
→ Decision: Favor readable code that demonstrates library features clearly

## Migration Plan

Not applicable - this is a new demo application with no migration required.

## Open Questions

None - the design is straightforward and uses existing library features.
