## 1. Setup and Structure

- [x] 1.1 Create demo directory structure at `demos/log-analyzer/`
- [x] 1.2 Create Windows build script `build-windows-amd64.bat`
- [x] 1.3 Create Linux build script `build-linux-amd64.sh`
- [x] 1.4 Add demo to demos/README.md table

## 2. Core Log Parsing

- [x] 2.1 Implement async file reading using `fun_read_file_in_memory()`
- [x] 2.2 Implement line-by-line parsing to extract timestamp, level, and message
- [x] 2.3 Handle log format `[TIMESTAMP] [LEVEL] message`
- [x] 2.4 Add error handling for malformed lines and file read errors

## 3. Statistics Collection

- [x] 3.1 Create HashMap for counting log levels (TRACE, DEBUG, INFO, WARN, ERROR)
- [x] 3.2 Implement level extraction and counting logic
- [x] 3.3 Create time bucket aggregation (by hour and minute)
- [x] 3.4 Implement pattern filtering using `fun_string_index_of()`

## 4. Output and Display

- [x] 4.1 Create formatted console output using string templates
- [x] 4.2 Display level counts in table format
- [x] 4.3 Display time distribution histogram
- [x] 4.4 Implement optional file output for detailed results

## 5. Command-Line Interface

- [x] 5.1 Parse command-line arguments (file path, pattern filter, output file)
- [x] 5.2 Add help text and usage examples
- [x] 5.3 Validate input parameters and provide clear error messages

## 6. Testing and Validation

- [x] 6.1 Create sample log file for testing
- [x] 6.2 Test on Windows AMD64 platform
- [x] 6.3 Test on Linux AMD64 platform
- [x] 6.4 Run code formatter (`clang-format`)
- [x] 6.5 Run demo validator script

## 7. Documentation

- [x] 7.1 Add README.md with usage examples
- [x] 7.2 Document expected log format
- [x] 7.3 List all command-line options with examples
