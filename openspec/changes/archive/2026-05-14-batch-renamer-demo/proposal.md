## Why

File management tasks like batch renaming are common operations for developers, system administrators, and end users. A batch renamer demo showcases the Fundamental Library's filesystem operations, string manipulation, and user interaction patterns in a practical tool that provides immediate value. This demonstrates real-world applicability while exercising multiple library modules.

## What Changes

- **New demo application** in `demos/batch-renamer/` with complete build scripts for Windows and Linux
- **Multiple renaming modes**: find/replace, prefix/suffix addition, sequential numbering, extension changes
- **Pattern matching** with wildcard support for selecting files (*.txt, log_*.bak, etc.)
- **Dry-run mode** to preview changes before applying them
- **Safe operation** with error handling for conflicts and permission issues

## Capabilities

### New Capabilities

- `batch-rename`: Rename multiple files with various strategies (find/replace, prefix/suffix, numbering, extension changes), support wildcards, and provide dry-run preview
- `filesystem-enumeration`: List directory contents, filter by patterns, and handle file operations safely

### Modified Capabilities

<!-- No existing capabilities are being modified - this is a new demo application -->

## Impact

- **New demo directory**: `demos/batch-renamer/` with demo.c and build scripts
- **Dependencies**: Filesystem module (directory listing, path operations, file exists checks), String module (manipulation, templates), Console module (I/O), Collections (array for file lists)
- **No breaking changes**: This is additive functionality that doesn't affect existing code
- **Platform support**: Windows AMD64 and Linux AMD64 (following existing demo patterns)
- **User interaction**: Demonstrates console input/output for interactive renaming workflow
