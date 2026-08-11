# Specifications

## ADDED Requirements

### Requirement: Monotonic nanosecond clock

The system SHALL provide `fun_timing_now_ns()` that returns a `uint64_t` of nanoseconds from a monotonic clock. The function SHALL NOT require initialization. The clock SHALL be monotonic (never decrease between successive calls on the same thread). The value SHALL be suitable for measuring elapsed time as the difference between two calls.

#### Scenario: Monotonic property

- **WHEN** `fun_timing_now_ns()` is called twice in succession on the same thread
- **THEN** the second return value SHALL be greater than or equal to the first

#### Scenario: Platform-independent header

- **WHEN** `include/fundamental/timing/timing.h` is included on any supported platform
- **THEN** the `fun_timing_now_ns` function SHALL be declared and usable after linking the platform-specific arch source

### Requirement: Windows implementation uses QueryPerformanceCounter

On Windows-amd64, `fun_timing_now_ns` SHALL use `QueryPerformanceCounter` and `QueryPerformanceFrequency`. The frequency SHALL be cached on first call.

#### Scenario: Returns positive value on Windows

- **WHEN** `fun_timing_now_ns()` is called on Windows-amd64
- **THEN** the return value SHALL be greater than 0

### Requirement: Linux implementation uses clock_gettime

On Linux-amd64, `fun_timing_now_ns` SHALL use `clock_gettime(CLOCK_MONOTONIC, ...)`. No initialization or caching SHALL be required.

#### Scenario: Returns positive value on Linux

- **WHEN** `fun_timing_now_ns()` is called on Linux-amd64
- **THEN** the return value SHALL be greater than 0
