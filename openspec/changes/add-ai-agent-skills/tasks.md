## 1. Skill File Structure

- [x] 1.1 Create `.opencode/skills/fundamental-file-io.md` with YAML frontmatter
- [x] 1.2 Create `.opencode/skills/fundamental-memory.md` with YAML frontmatter
- [x] 1.3 Create `.opencode/skills/fundamental-console.md` with YAML frontmatter
- [x] 1.4 Create `.opencode/skills/fundamental-directory.md` with YAML frontmatter
- [x] 1.5 Create `.opencode/skills/fundamental-string.md` with YAML frontmatter
- [x] 1.6 Create `.opencode/skills/fundamental-collections.md` with YAML frontmatter
- [x] 1.7 Create `.opencode/skills/fundamental-async.md` with YAML frontmatter
- [x] 1.8 Create `.opencode/skills/fundamental-config.md` with YAML frontmatter

## 2. File I/O Skill Content

- [x] 2.1 Add file read example with memory allocation and async wait
- [x] 2.2 Add file write example with buffer preparation
- [x] 2.3 Add file existence check example (fun_file_exists when available)
- [x] 2.4 Add file append example
- [x] 2.5 Add error handling section with common file error codes
- [x] 2.6 Add memory cleanup examples for all file operations
- [x] 2.7 Add stream-based file reading example for large files

## 3. Memory Skill Content

- [x] 3.1 Add memory allocation example with error checking
- [x] 3.2 Add memory free example with pointer handling explanation
- [x] 3.3 Add memory copy example (fun_memory_copy)
- [x] 3.4 Add memory fill example (fun_memory_fill)
- [x] 3.5 Add memory compare example (fun_memory_compare)
- [x] 3.6 Add memory safety section: no use-after-free, no double-free
- [x] 3.7 Add buffer sizing guidelines for common scenarios

## 4. Console Skill Content

- [x] 4.1 Add basic output examples (fun_console_write, fun_console_write_line)
- [x] 4.2 Add error output example (fun_console_error_line)
- [x] 4.3 Add console flush example (fun_console_flush)
- [x] 4.4 Add progress bar implementation example
- [x] 4.5 Add progress bar update pattern in loop
- [x] 4.6 Add console output buffering explanation

## 5. Directory Skill Content

- [x] 5.1 Add directory create example with error handling
- [x] 5.2 Add directory list example with buffer allocation
- [x] 5.3 Add directory listing parsing example (newline-separated)
- [x] 5.4 Add directory remove example with emptiness check
- [x] 5.5 Add directory existence check example (fun_directory_exists)
- [x] 5.6 Add directory iteration pattern (list → parse → iterate)
- [x] 5.7 Add cleanup examples for directory operations

## 6. String Skill Content

- [x] 6.1 Add string copy example (fun_string_copy)
- [x] 6.2 Add string join example (fun_string_join)
- [x] 6.3 Add string template example with parameters
- [x] 6.4 Add string conversion examples (int to string, double to string)
- [x] 6.5 Add string trim example (fun_string_trim_in_place)
- [x] 6.6 Add string comparison example (fun_string_compare)
- [x] 6.7 Add string length example (fun_string_length)

## 7. Collections Skill Content

- [x] 7.1 Add dynamic array type definition example (DEFINE_ARRAY_TYPE)
- [x] 7.2 Add array create/push/get/pop example
- [x] 7.3 Add array iteration example
- [x] 7.4 Add hashmap string-key example with put/get
- [x] 7.5 Add hashmap iteration example
- [x] 7.6 Add set creation and membership test example
- [x] 7.7 Add collection cleanup/destroy examples for all types

## 8. Async Skill Content

- [x] 8.1 Add async result await example (fun_async_await)
- [x] 8.2 Add async status check pattern (ASYNC_PENDING, ASYNC_COMPLETED, ASYNC_ERROR)
- [x] 8.3 Add async poll pattern for non-blocking operations
- [x] 8.4 Add process spawn example with output capture
- [x] 8.5 Add async error handling pattern

## 9. Config Skill Content

- [x] 9.1 Add config load example (fun_config_load)
- [x] 9.2 Add required config value example with explicit error check
- [x] 9.3 Add optional config value example with default
- [x] 9.4 Add config cascade explanation (CLI → env → INI)
- [x] 9.5 Add config destroy example

## 10. Cross-References and Links

- [x] 10.1 Add cross-references between related skills (memory ↔ file, console ↔ string)
- [x] 10.2 Add links to header files for complete API reference
- [x] 10.3 Add "See Also" sections in each skill
- [x] 10.4 Add index skill or README that lists all skills

## 11. Agent Integration

- [x] 11.1 Test skill discovery with Opencode queries
- [x] 11.2 Test skill discovery with Claude Code queries
- [x] 11.3 Update agent system prompts if needed to reference skills
- [x] 11.4 Gather feedback on skill usefulness and clarity

## 12. Documentation

- [x] 12.1 Add skills index to project README
- [x] 12.2 Add "Skills for AI Agents" section to CONTRIBUTING.md (via AGENTS.md update)
- [x] 12.3 Document skill format for future skill authors
- [x] 12.4 Add version metadata to all skill files
