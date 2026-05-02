## 1. Setup and Structure

- [ ] 1.1 Create demo directory structure at `demos/batch-renamer/`
- [ ] 1.2 Create Windows build script `build-windows-amd64.bat`
- [ ] 1.3 Create Linux build script `build-linux-amd64.sh`
- [ ] 1.4 Add demo to demos/README.md table

## 2. File Enumeration

- [ ] 2.1 Implement directory listing using `fun_filesystem_list_directory()`
- [ ] 2.2 Implement wildcard pattern matching (* and ?)
- [ ] 2.3 Filter files based on pattern
- [ ] 2.4 Store matching files in Array data structure

## 3. Rename Transformations

- [ ] 3.1 Implement find-and-replace transformation
- [ ] 3.2 Implement prefix addition transformation
- [ ] 3.3 Implement suffix addition transformation
- [ ] 3.4 Implement sequential numbering with zero-padding
- [ ] 3.5 Implement extension change transformation
- [ ] 3.6 Support combining multiple transformations

## 4. Conflict Detection

- [ ] 4.1 Check for existing files using `fun_file_exists()`
- [ ] 4.2 Detect self-references (rename to same name)
- [ ] 4.3 Report conflicts in preview mode
- [ ] 4.4 Handle conflicts gracefully in apply mode

## 5. Dry-Run and Apply Modes

- [ ] 5.1 Implement dry-run preview showing before/after names
- [ ] 5.2 Implement actual file renaming with `fun_filesystem_rename()` (or platform equivalent)
- [ ] 5.3 Add `--apply` flag to enable actual renaming
- [ ] 5.4 Default to dry-run mode for safety

## 6. Command-Line Interface

- [ ] 6.1 Parse command-line arguments for all modes and options
- [ ] 6.2 Add help text with usage examples for each mode
- [ ] 6.3 Validate input parameters
- [ ] 6.4 Provide clear error messages for invalid inputs

## 7. Testing and Validation

- [ ] 7.1 Create test directory with sample files
- [ ] 7.2 Test each rename mode individually on Windows
- [ ] 7.3 Test each rename mode individually on Linux
- [ ] 7.4 Test conflict detection and error handling
- [ ] 7.5 Run code formatter (`clang-format`)
- [ ] 7.6 Run demo validator script

## 8. Documentation

- [ ] 8.1 Add README.md with usage examples
- [ ] 8.2 Document all rename modes with examples
- [ ] 8.3 Document wildcard pattern syntax
- [ ] 8.4 List all command-line options
