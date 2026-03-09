## Why

The Fundamental Library lacks a unified configuration management system for CLI applications, forcing developers to manually parse command-line arguments, environment variables, and INI files using stdlib functions. This change adds a cascading config module that enables zero-stdlib CLI applications with ergonomic access to configuration from multiple sources.

## What Changes

- New config module with cascading configuration sources (CLI → Environment → INI file)
- Type-safe getters with explicit error handling for required values
- Ergonomic `get_or_default()` variants for optional configuration
- INI file parser using existing file module (no platform-specific code)
- Platform-specific environment variable access in `arch/config/*/`
- Command-line argument parsing with `--config:key=value` prefix
- No breaking changes to existing APIs

## Capabilities

### New Capabilities
- `config`: Cascading configuration management with support for command-line arguments, environment variables, and INI file sources with type-safe accessors

### Modified Capabilities
- None

## Impact

- New public API in `include/config/config.h` with `fun_config_*` functions
- New source files in `src/config/` for core logic and INI parsing
- Platform-specific code in `arch/config/windows-amd64/` and `arch/config/linux-amd64/` for environment variable access
- INI parser implemented in `src/config/` using `fun_read_file_in_memory()` from file module
- Enables CLI application development without stdlib runtime dependencies
- No breaking changes to existing modules
