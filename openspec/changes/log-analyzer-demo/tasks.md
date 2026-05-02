## 1. Setup and Structure

- [ ] 1.1 Create demo directory structure at `demos/log-analyzer/`
- [ ] 1.2 Create Windows build script `build-windows-amd64.bat`
- [ ] 1.3 Create Linux build script `build-linux-amd64.sh`
- [ ] 1.4 Add demo to demos/README.md table

## 2. Core Log Parsing

- [ ] 2.1 Implement async file reading using `fun_read_file_in_memory()`
- [ ] 2.2 Implement line-by-line parsing to extract timestamp, level, and message
- [ ] 2.3 Handle log format `[TIMESTAMP] [LEVEL] message`
- [ ] 2.4 Add error handling for malformed lines and file read errors

## 3. Statistics Collection

- [ ] 3.1 Create HashMap for counting log levels (TRACE, DEBUG, INFO, WARN, ERROR)
- [ ] 3.2 Implement level extraction and counting logic
- [ ] 3.3 Create time bucket aggregation (by hour and minute)
- [ ] 3.4 Implement pattern filtering using `fun_string_index_of()`

## 4. Output and Display

- [ ] 4.1 Create formatted console output using string templates
- [ ] 4.2 Display level counts in table format
- [ ] 4.3 Display time distribution histogram
- [ ] 4.4 Implement optional file output for detailed results

## 5. Command-Line Interface

- [ ] 5.1 Parse command-line arguments (file path, pattern filter, output file)
- [ ] 5.2 Add help text and usage examples
- [ ] 5.3 Validate input parameters and provide clear error messages

## 6. Testing and Validation

- [ ] 6.1 Create sample log file for testing
- [ ] 6.2 Test on Windows AMD64 platform
- [ ] 6.3 Test on Linux AMD64 platform
- [ ] 6.4 Run code formatter (`clang-format`)
- [ ] 6.5 Run demo validator script

## 7. Documentation

- [ ] 7.1 Add README.md with usage examples
- [ ] 7.2 Document expected log format
- [ ] 7.3 List all command-line options with examples
