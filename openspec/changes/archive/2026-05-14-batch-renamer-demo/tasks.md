## 1. Setup and Structure

- [x] 1.1 Create demo directory structure at `demos/batch-renamer/`
- [x] 1.2 Create Windows build script `build-windows-amd64.bat`
- [x] 1.3 Create Linux build script `build-linux-amd64.sh`
- [x] 1.4 Add demo to demos/README.md table

## 2. File Enumeration

- [x] 2.1 Implement directory listing using `fun_filesystem_list_directory()`
- [x] 2.2 Implement wildcard pattern matching (* and ?)
- [x] 2.3 Filter files based on pattern
- [x] 2.4 Store matching files in Array data structure

## 3. Rename Transformations

- [x] 3.1 Implement find-and-replace transformation
- [x] 3.2 Implement prefix addition transformation
- [x] 3.3 Implement suffix addition transformation
- [x] 3.4 Implement sequential numbering with zero-padding
- [x] 3.5 Implement extension change transformation
- [x] 3.6 Support combining multiple transformations

## 4. Conflict Detection

- [x] 4.1 Check for existing files using `fun_file_exists()`
- [x] 4.2 Detect self-references (rename to same name)
- [x] 4.3 Report conflicts in preview mode
- [x] 4.4 Handle conflicts gracefully in apply mode

## 5. Dry-Run and Apply Modes

- [x] 5.1 Implement dry-run preview showing before/after names
- [x] 5.2 Implement actual file renaming with `fun_filesystem_rename()` (or platform equivalent)
- [x] 5.3 Add `--apply` flag to enable actual renaming
- [x] 5.4 Default to dry-run mode for safety

## 6. Command-Line Interface

- [x] 6.1 Parse command-line arguments for all modes and options
- [x] 6.2 Add help text with usage examples for each mode
- [x] 6.3 Validate input parameters
- [x] 6.4 Provide clear error messages for invalid inputs

## 7. Testing and Validation

- [x] 7.1 Create test directory with sample files
- [x] 7.2 Test each rename mode individually on Windows
- [x] 7.3 Test each rename mode individually on Linux
- [x] 7.4 Test conflict detection and error handling
- [x] 7.5 Run code formatter (`clang-format`)
- [x] 7.6 Run demo validator script

## 8. Documentation

- [x] 8.1 Add README.md with usage examples
- [x] 8.2 Document all rename modes with examples
- [x] 8.3 Document wildcard pattern syntax
- [x] 8.4 List all command-line options
