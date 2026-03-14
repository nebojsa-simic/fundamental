## Why

Current path handling relies on platform-specific separator characters ('/' vs '\\') passed as strings throughout the API, creating cross-platform complexity and potential bugs. Developers must understand separator handling, leading to brittle code when paths are constructed, parsed, or manipulated across different operating systems.

## What Changes

- **BREAKING**: Replace `String path` parameters with new `Path` type throughout filesystem API
- Introduce `Path` as array-of-strings structure representing path components
- Add `fun_path_from_string(String path, Path *output)` for string-to-Path conversion
- Add `fun_path_to_string(Path path, OutputString output, size_t buffer_size)` for Path-to-string conversion
- Retain `fun_path_separator()` for use cases requiring separator character access
- Refactor `fun_path_join()`, `fun_path_normalize()`, `fun_path_get_parent()`, `fun_path_get_filename()` to work with `Path` type
- Update all filesystem functions (`create_directory`, `remove_directory`, `list_directory`) to accept `Path`
- **BREAKING**: Change function signatures - existing string-based calls will fail compilation

## Capabilities

### New Capabilities
- `path-type`: New Path type definition with component-based representation, conversion utilities, and manipulation operations

### Modified Capabilities
- `filesystem`: All path-related functions now use Path type instead of String, changing function signatures and behavior

## Impact

- **Breaking API change**: All existing filesystem API calls using String paths must migrate to Path type
- **Affected functions**: `fun_path_join`, `fun_path_normalize`, `fun_path_get_parent`, `fun_path_get_filename`, `fun_filesystem_create_directory`, `fun_filesystem_remove_directory`, `fun_filesystem_list_directory`
- **New functions**: `fun_path_from_string()`, `fun_path_to_string()` for string↔Path conversion
- **Unchanged**: `fun_path_separator()` retained for manual string construction use cases
- **File module**: File I/O functions accepting paths will need migration (future change)
- **Architecture layer**: Platform-specific code in `arch/filesystem/*/` must convert Path to native strings for system calls
- **Tests**: All filesystem tests require updates to use Path type
- **Migration required**: Existing code must convert string paths to Path type at API boundaries
