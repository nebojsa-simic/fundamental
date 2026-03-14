# platform Specification

## Purpose
Provides compile-time OS and architecture detection via the arch layer.
Each supported platform supplies `fun_platform_os()` and `fun_platform_arch()`
in its `arch/platform/<platform>/` directory, eliminating `#ifdef` from
shared code.

## Requirements

### Requirement: Platform Detection
The platform module SHALL provide a function to retrieve the current OS and architecture.

#### Scenario: Get platform on Windows AMD64
- **WHEN** `fun_platform_get(&p)` is called on a Windows AMD64 build
- **THEN** `p.os == PLATFORM_OS_WINDOWS` and `p.arch == PLATFORM_ARCH_AMD64`

#### Scenario: Get platform on Linux AMD64
- **WHEN** `fun_platform_get(&p)` is called on a Linux AMD64 build
- **THEN** `p.os == PLATFORM_OS_LINUX` and `p.arch == PLATFORM_ARCH_AMD64`

#### Scenario: Get platform on Linux ARM64
- **WHEN** `fun_platform_get(&p)` is called on a Linux ARM64 build
- **THEN** `p.os == PLATFORM_OS_LINUX` and `p.arch == PLATFORM_ARCH_ARM64`

#### Scenario: Get platform on Darwin ARM64
- **WHEN** `fun_platform_get(&p)` is called on a Darwin ARM64 build
- **THEN** `p.os == PLATFORM_OS_DARWIN` and `p.arch == PLATFORM_ARCH_ARM64`

### Requirement: OS String Conversion
The platform module SHALL provide a function to convert a `PlatformOS` value to a string.

#### Scenario: Convert PLATFORM_OS_WINDOWS
- **WHEN** `fun_platform_os_to_string(PLATFORM_OS_WINDOWS, buf)` is called
- **THEN** `buf` contains `"windows"` and error is `ERROR_CODE_NO_ERROR`

#### Scenario: Convert PLATFORM_OS_LINUX
- **WHEN** `fun_platform_os_to_string(PLATFORM_OS_LINUX, buf)` is called
- **THEN** `buf` contains `"linux"` and error is `ERROR_CODE_NO_ERROR`

#### Scenario: Convert PLATFORM_OS_DARWIN
- **WHEN** `fun_platform_os_to_string(PLATFORM_OS_DARWIN, buf)` is called
- **THEN** `buf` contains `"darwin"` and error is `ERROR_CODE_NO_ERROR`

#### Scenario: Convert PLATFORM_OS_UNKNOWN
- **WHEN** `fun_platform_os_to_string(PLATFORM_OS_UNKNOWN, buf)` is called
- **THEN** `buf` contains `"unknown"` and error is `ERROR_CODE_NO_ERROR`

#### Scenario: NULL output buffer
- **WHEN** `fun_platform_os_to_string(os, NULL)` is called
- **THEN** the function returns an error with `ERROR_CODE_NULL_POINTER`

### Requirement: Architecture String Conversion
The platform module SHALL provide a function to convert a `PlatformArch` value to a string.

#### Scenario: Convert PLATFORM_ARCH_AMD64
- **WHEN** `fun_platform_arch_to_string(PLATFORM_ARCH_AMD64, buf)` is called
- **THEN** `buf` contains `"amd64"` and error is `ERROR_CODE_NO_ERROR`

#### Scenario: Convert PLATFORM_ARCH_ARM64
- **WHEN** `fun_platform_arch_to_string(PLATFORM_ARCH_ARM64, buf)` is called
- **THEN** `buf` contains `"arm64"` and error is `ERROR_CODE_NO_ERROR`

#### Scenario: Convert PLATFORM_ARCH_UNKNOWN
- **WHEN** `fun_platform_arch_to_string(PLATFORM_ARCH_UNKNOWN, buf)` is called
- **THEN** `buf` contains `"unknown"` and error is `ERROR_CODE_NO_ERROR`

#### Scenario: NULL output buffer
- **WHEN** `fun_platform_arch_to_string(arch, NULL)` is called
- **THEN** the function returns an error with `ERROR_CODE_NULL_POINTER`

### Requirement: Full Platform String
The platform module SHALL provide a function to write the combined `"os-arch"` string.

#### Scenario: Format is os-arch
- **WHEN** `fun_platform_to_string(platform, buf)` is called
- **THEN** `buf` contains `"<os>-<arch>"` (e.g. `"windows-amd64"`) and error is `ERROR_CODE_NO_ERROR`

#### Scenario: NULL output buffer
- **WHEN** `fun_platform_to_string(platform, NULL)` is called
- **THEN** the function returns an error with `ERROR_CODE_NULL_POINTER`
