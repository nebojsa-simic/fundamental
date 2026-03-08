## Why

The Fundamental library provides file I/O operations but lacks directory management and path manipulation capabilities. Applications need to create directories, list directory contents, and handle cross-platform path operations without relying on stdlib functions or platform-specific code. This change adds a filesystem module to complement the existing file module.

## What Changes

- New filesystem module with directory operations (create, list, remove)
- Path management utilities for cross-platform path handling
- Platform abstraction for directory operations (Windows API, POSIX syscalls)
- No stdlib runtime functions used in implementation
- No breaking changes to existing APIs

## Capabilities

### New Capabilities
- `filesystem`: Directory operations (create directory, list directory contents, remove directory) and path management utilities (join, normalize, extract components)

### Modified Capabilities
- None

## Impact

- New public API in `include/filesystem/filesystem.h` with fun_filesystem_* functions
- New source files in `src/filesystem/` for directory and path operations
- Platform-specific code in `arch/filesystem/windows-amd64/` and `arch/filesystem/linux-amd64/`
- Complements existing file module without modifying file I/O APIs
- No breaking changes to existing modules
