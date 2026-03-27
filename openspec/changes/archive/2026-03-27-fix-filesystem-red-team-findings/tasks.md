## 1. Add ERROR_CODE_PATH_TOO_LONG

- [x] 1.1 Open `include/fundamental/filesystem/filesystem.h` and check whether `ERROR_CODE_PATH_TOO_LONG` already exists
- [x] 1.2 If absent, add `ERROR_CODE_PATH_TOO_LONG` to the error code enum / defines alongside the other `ERROR_CODE_*` constants

## 2. Fix path_to_string_internal (silent truncation)

- [x] 2.1 In `src/filesystem/directory.c`, locate `path_to_string_internal`
- [x] 2.2 Before writing each path component, check that `out_pos + component_len + separator + null` fits within `buffer_size`
- [x] 2.3 If the check fails, return `fun_error_result(ERROR_CODE_PATH_TOO_LONG, "Path exceeds buffer")` instead of continuing
- [x] 2.4 Verify all callers of `path_to_string_internal` already propagate `ErrorResult` — no further changes needed if they do

## 3. Fix create_parent_directories separator

- [x] 3.1 In `src/filesystem/path.c` around line 161, replace the hardcoded `'/'` restore with `fun_path_separator()`

## 4. Fix fun_filesystem_remove_directory error code mapping

- [x] 4.1 In `src/filesystem/directory.c` in `fun_filesystem_remove_directory`, locate the `if (result < 0)` catch-all
- [x] 4.2 Add an explicit `else if (result == -1)` branch that returns `fun_error_result(ERROR_CODE_DIRECTORY_NOT_EMPTY, "Directory is not empty")`
- [x] 4.3 Ensure this branch appears before the generic `result < 0` branch

## 5. Fix fun_platform_directory_is_empty on open failure

- [x] 5.1 In `arch/filesystem/linux-amd64/directory.c`, locate `fun_platform_directory_is_empty`
- [x] 5.2 Change the open-failure path to `return 0` (false/not-empty) instead of `return 1` (true/empty)
- [x] 5.3 In `arch/filesystem/windows-amd64/directory.c`, locate `fun_platform_directory_is_empty` (or equivalent)
- [x] 5.4 Apply the same fix: return 0 (false) on open/FindFirstFile failure

## 6. Document walk long-path skip behaviour

- [x] 6.1 In `src/filesystem/walk.c`, locate the `continue` at the `walk_build_child_path` failure site (around line 246-248)
- [x] 6.2 Add a brief comment explaining the skip is intentional: entry path exceeds `FUN_WALK_PATH_SIZE`, entry is skipped per spec

## 7. Tests

- [x] 7.1 In `tests/filesystem/test_filesystem.c`, add `test_path_too_long`: construct a Path with enough components to exceed 512 bytes and verify `fun_filesystem_create_directory` returns `ERROR_CODE_PATH_TOO_LONG`
- [x] 7.2 Add `test_remove_directory_not_empty_error_code`: create a directory with a file inside, call `fun_filesystem_remove_directory`, verify the error code is `ERROR_CODE_DIRECTORY_NOT_EMPTY` (not `ERROR_CODE_PERMISSION_DENIED`)
- [x] 7.3 Build and run the filesystem test suite on Linux: `cd tests/filesystem && ./build-linux-amd64.sh && ./test`
- [x] 7.4 Build and run the filesystem test suite on Windows: `cd tests/filesystem && build-windows-amd64.bat && test.exe`

## 8. Format

- [x] 8.1 Run `./code-format.sh` (Linux) or `code-format.bat` (Windows) to apply clang-format to all changed files
