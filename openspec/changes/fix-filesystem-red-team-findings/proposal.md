## Why

The filesystem module contains critical and high-severity defects identified in a red team review: path strings are silently truncated at 512 bytes without returning an error, a wrong separator character is hardcoded in `create_parent_directories`, directory-walk silently skips entries with long paths, `fun_filesystem_remove_directory` maps the wrong error code, and `fun_platform_directory_is_empty` treats an unopenable directory as empty. These bugs can cause silent data loss, wrong-path operations, and misleading error messages in production use.

## What Changes

- `path_to_string_internal` returns an error when the path would exceed the buffer, instead of silently truncating
- `create_parent_directories` uses `fun_path_separator()` instead of hardcoded `'/'`
- `fun_filesystem_walk_next` reports an error entry (or skips with a logged reason) when a child path exceeds `FUN_WALK_PATH_SIZE`, instead of silently continuing
- `fun_filesystem_remove_directory` platform call return code `-1` (not-empty) is mapped to `ERROR_CODE_DIRECTORY_NOT_EMPTY` instead of `ERROR_CODE_PERMISSION_DENIED`
- `fun_platform_directory_is_empty` returns an error (not `true`) when the directory cannot be opened
- `fun_filesystem_get_working_directory` documents the 512-byte ceiling and returns `ERROR_CODE_PATH_TOO_LONG` if the buffer would overflow

## Capabilities

### New Capabilities

*(none — all changes are defect fixes within existing capabilities)*

### Modified Capabilities

- `filesystem`: Correct error propagation for path truncation, wrong separator, bad error codes, and empty-check on inaccessible directory
- `filesystem-walk`: Silent skip of long paths changed to explicit error/skip behaviour with correct error propagation

## Impact

- `src/filesystem/directory.c` — `path_to_string_internal`, `create_parent_directories`, `fun_filesystem_remove_directory`
- `src/filesystem/walk.c` — `walk_build_child_path` caller, `fun_filesystem_walk_next`
- `arch/filesystem/linux-amd64/directory.c` — `fun_platform_directory_is_empty`
- `arch/filesystem/windows-amd64/directory.c` — `fun_platform_directory_is_empty`
- `include/fundamental/filesystem/filesystem.h` — new error code `ERROR_CODE_PATH_TOO_LONG` (if not already present)
- `tests/filesystem/test_filesystem.c` — new test cases covering each fixed path
- No API additions; all changes are error-propagation corrections or documentation of existing limits
