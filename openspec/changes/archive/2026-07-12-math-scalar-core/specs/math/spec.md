## ADDED Requirements

### Requirement: Public API header
The system SHALL provide `include/fundamental/math/math.h` that declares all scalar and vector math functions with their complete signatures.

#### Scenario: Header declares scalar functions
- **WHEN** the header is included
- **THEN** the following functions are declared: `fun_math_sqrt(float)`, `fun_math_exp(float)`, `fun_math_log(float)`, `fun_math_sin(float)`, `fun_math_cos(float)`, `fun_math_tanh(float)`, `fun_math_sigmoid(float)`, `fun_math_silu(float)`

#### Scenario: Header declares vector functions
- **WHEN** the header is included
- **THEN** the following functions are declared: `fun_math_silu_f32`, `fun_math_rms_norm_f32`, `fun_math_swiglu_f32`, `fun_math_softmax_f32`

### Requirement: sqrt implementation
The system SHALL implement `fun_math_sqrt` using fast inverse square root with Newton-Raphson refinement, achieving relative error below 1e-4 across the positive input range.

#### Scenario: sqrt of positive values
- **WHEN** sqrt is called with inputs in [0, 1e6]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-4 relative error

#### Scenario: sqrt of zero
- **WHEN** sqrt is called with +0.0
- **THEN** it returns +0.0
- **WHEN** sqrt is called with -0.0
- **THEN** it returns -0.0

#### Scenario: sqrt of negative
- **WHEN** sqrt is called with a negative finite value
- **THEN** it returns NaN

#### Scenario: sqrt of NaN
- **WHEN** sqrt is called with NaN
- **THEN** it returns NaN

### Requirement: exp implementation
The system SHALL implement `fun_math_exp` using range reduction to `[0, 1)` with a rational Pade approximant, achieving relative error below 1e-3 across the input range [-88, 88].

#### Scenario: exp normal range
- **WHEN** exp is called with inputs in [-20, 20]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: exp overflow
- **WHEN** exp is called with input greater than approximately 88.7
- **THEN** it returns +inf

#### Scenario: exp underflow
- **WHEN** exp is called with input less than approximately -87
- **THEN** it returns +0.0

#### Scenario: exp special values
- **WHEN** exp is called with +inf
- **THEN** it returns +inf
- **WHEN** exp is called with -inf
- **THEN** it returns +0.0
- **WHEN** exp is called with NaN
- **THEN** it returns NaN

### Requirement: log implementation
The system SHALL implement `fun_math_log` using exponent/mantissa extraction with Newton iteration on the mantissa, achieving relative error below 1e-3 across the positive input range.

#### Scenario: log normal range
- **WHEN** log is called with inputs in [0.001, 1e6]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: log special values
- **WHEN** log is called with 1.0
- **THEN** it returns 0.0
- **WHEN** log is called with +0.0 or -0.0
- **THEN** it returns -inf
- **WHEN** log is called with a negative finite value
- **THEN** it returns NaN
- **WHEN** log is called with +inf
- **THEN** it returns +inf
- **WHEN** log is called with NaN
- **THEN** it returns NaN

### Requirement: CPU feature detection
The system SHALL detect available CPU SIMD features at startup and store the results for later dispatch.

#### Scenario: Feature detection on x86-64
- **WHEN** `fun_math_init()` is called during startup Phase 1
- **THEN** the CPUID instruction is executed and SSE2/AVX/AVX2/AVX-512F support flags are recorded in a static structure accessible via `fun_math_has_sse2()`, `fun_math_has_avx()`, `fun_math_has_avx2()`, `fun_math_has_avx512f()`

#### Scenario: Feature query returns false when unsupported
- **WHEN** `fun_math_has_avx512f()` is called on a CPU without AVX-512
- **THEN** it returns 0 (false)

### Requirement: Unimplemented functions return sentinel
Functions deferred to future chunks SHALL return 0.0 so that tests can compile and run, making failures clearly attributable to missing implementations.

#### Scenario: Deferred scalar functions
- **WHEN** `fun_math_sin`, `fun_math_cos`, `fun_math_tanh`, `fun_math_sigmoid`, or `fun_math_silu` is called
- **THEN** it returns 0.0

## MODIFIED Requirements

### Requirement: Build scripts compile test binary
The system SHALL provide platform-specific build scripts that compile the test harness into an executable using gcc with the Fundamental Library's build conventions. For this chunk (no SIMD), the build SHALL not require AVX-512 flags.

#### Scenario: Windows amd64 build
- **WHEN** `build-windows-amd64.bat` is executed
- **THEN** it compiles all test source files plus `src/math/math_scalar.c` and `arch/math/windows-amd64/cpu_features.c` into `test.exe` using `gcc --std=c17 -Os -I ../../include`, strips the binary, and exits with code 0

#### Scenario: Linux amd64 build
- **WHEN** `build-linux-amd64.sh` is executed
- **THEN** it compiles all test source files plus `src/math/math_scalar.c` and `arch/math/linux-amd64/cpu_features.c` into `test` using `gcc --std=c17 -Os -I ../../include`, strips the binary, and exits with code 0
