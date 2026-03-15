## ADDED Requirements

### Requirement: Arch layer provides monotonic millisecond clock
Each platform arch implementation SHALL provide `arch_async_now_ms()` returning the current monotonic time in milliseconds as an `unsigned long long`. The `src/async/async.c` module SHALL call this function for all timeout calculations and SHALL NOT contain any `#ifdef` platform guards or OS-specific includes.

#### Scenario: Windows implementation returns tick count
- **WHEN** `arch_async_now_ms()` is called on a Windows AMD64 build
- **THEN** it returns `(unsigned long long)GetTickCount64()`

#### Scenario: Linux implementation returns clock_gettime result
- **WHEN** `arch_async_now_ms()` is called on a Linux AMD64 build
- **THEN** it returns `tv_sec * 1000 + tv_nsec / 1000000` from `clock_gettime(CLOCK_MONOTONIC)`

#### Scenario: src/async/async.c contains no platform guards
- **WHEN** `src/async/async.c` is inspected
- **THEN** it contains no `#ifdef _WIN32`, no `<windows.h>`, no `<time.h>` includes
