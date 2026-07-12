# math Specification

## Purpose
TBD - created by archiving change math-module-tests. Update Purpose after archive.
## Requirements
### Requirement: Golden value generation tool
The system SHALL provide a Python tool at `tests/math/tools/generate_golden.py` that generates C header files containing precomputed test vectors. Each header SHALL define a struct type with `input`, `expected`, and `tolerance` fields, followed by a static const array of test cases terminated by a sentinel.

#### Scenario: Generate sqrt golden values
- **WHEN** the tool is invoked with `--function sqrt`
- **THEN** it writes `tests/math/test_data/sqrt_golden.h` containing at least 2000 test cases with inputs spanning [0, 1e6], expected values computed via Python's `math.sqrt`, and a per-case tolerance of 1e-5 (relative) or 1e-6 (absolute near zero)

#### Scenario: Generate exp golden values
- **WHEN** the tool is invoked with `--function exp`
- **THEN** it writes `tests/math/test_data/exp_golden.h` containing at least 2000 test cases with inputs spanning [-88, 88], expected values computed via Python's `math.exp`, and a per-case tolerance of 1e-4 (relative)

#### Scenario: Generate silu golden values
- **WHEN** the tool is invoked with `--function silu`
- **THEN** it writes `tests/math/test_data/silu_golden.h` containing at least 2000 test cases with inputs spanning [-10, 10], expected values as `x * (1 / (1 + exp(-x)))` via Python's `math.exp`, and a per-case tolerance of 1e-5 (relative)

### Requirement: Scalar function accuracy tests
The system SHALL verify that each scalar math function produces results within tolerance of IEEE 754 reference values. Tests SHALL use golden values from generated headers and report pass/fail counts via `fun_console_write_line`.

#### Scenario: Scalar sqrt accuracy
- **WHEN** the sqrt accuracy test runs against `sqrt_golden.h`
- **THEN** every test case where the result differs from expected by more than tolerance is counted as failed, and the test reports the total pass and fail counts

#### Scenario: Scalar exp accuracy
- **WHEN** the exp accuracy test runs against `exp_golden.h`
- **THEN** every test case where the result differs from expected by more than tolerance is counted as failed, and the test reports the total pass and fail counts

#### Scenario: Scalar silu accuracy
- **WHEN** the silu accuracy test runs against `silu_golden.h`
- **THEN** every test case where the result differs from expected by more than tolerance is counted as failed, and the test reports the total pass and fail counts

#### Scenario: All scalar functions covered
- **WHEN** the full scalar accuracy suite runs
- **THEN** at minimum the following functions are tested: sqrt, exp, log, sin, cos, tanh, sigmoid, silu

### Requirement: Vector function accuracy tests
The system SHALL verify that vector (batch) math functions produce correct results when operating on arrays of floats. Tests SHALL use golden values from generated headers covering multi-element input/output patterns.

#### Scenario: Vector silu_f32 accuracy
- **WHEN** the silu_f32 accuracy test runs with arrays of length 1024 against precomputed golden values
- **THEN** every element where the result differs from expected by more than tolerance is counted as failed

#### Scenario: Vector rms_norm_f32 accuracy
- **WHEN** the rms_norm_f32 accuracy test runs with input/weight arrays of length 4096 against precomputed golden values
- **THEN** every output element is within tolerance of the expected normalized value

#### Scenario: Vector swiglu_f32 accuracy
- **WHEN** the swiglu_f32 accuracy test runs with gate/up arrays of length 1024 against precomputed golden values
- **THEN** every output element (gate * sigmoid(gate) * up) is within tolerance of expected

#### Scenario: Vector softmax_f32 accuracy
- **WHEN** the softmax_f32 accuracy test runs with an array of length 32 against precomputed golden values
- **THEN** the output probabilities sum to approximately 1.0 (within 1e-5) and each element matches expected within tolerance

### Requirement: Edge case handling tests
The system SHALL verify that math functions handle special floating-point values correctly: positive and negative zero, positive and negative infinity, NaN propagation, extreme magnitudes, and zero-length arrays.

#### Scenario: Zero inputs
- **WHEN** sqrt is called with -0.0
- **THEN** it returns -0.0 (preserving sign)
- **WHEN** sqrt is called with +0.0
- **THEN** it returns +0.0

#### Scenario: Infinity inputs
- **WHEN** exp is called with +inf
- **THEN** it returns +inf
- **WHEN** exp is called with -inf
- **THEN** it returns +0.0
- **WHEN** sqrt is called with +inf
- **THEN** it returns +inf

#### Scenario: NaN propagation
- **WHEN** any scalar math function is called with NaN input
- **THEN** it returns NaN

#### Scenario: Zero-length vector operations
- **WHEN** any vector function (silu_f32, rms_norm_f32, swiglu_f32, softmax_f32) is called with n=0
- **THEN** it returns immediately without accessing input or output arrays

#### Scenario: Extreme magnitude
- **WHEN** exp is called with the maximum finite float (approximately 3.4e38)
- **THEN** it returns +inf (saturating overflow)
- **WHEN** exp is called with approximately -88.7 or less
- **THEN** it returns +0.0 (saturating underflow)

### Requirement: Performance benchmarks
The system SHALL measure and report cycle counts for each math function using the RDTSC timestamp counter, operating over arrays large enough to amortize measurement overhead.

#### Scenario: Scalar function benchmark
- **WHEN** the sqrt performance benchmark runs
- **THEN** it reports cycles per element over at least 100,000 iterations on a 1024-element array

#### Scenario: Vector function benchmark
- **WHEN** the silu_f32 performance benchmark runs
- **THEN** it reports cycles per element over at least 10,000 iterations on a 65536-element array

#### Scenario: All functions benchmarked
- **WHEN** the full performance suite runs
- **THEN** it reports cycle counts for every scalar and vector function, with results written via `fun_console_write_line`

### Requirement: Test harness reports exit code
The system SHALL exit with code 0 when all tests pass and non-zero when any test fails. The test runner SHALL execute all test suites and report a summary before exiting.

#### Scenario: All tests pass
- **WHEN** every accuracy test, edge case test, and consistency test passes
- **THEN** the test process exits with code 0 and prints a summary showing zero failures

#### Scenario: Any test fails
- **WHEN** one or more test cases fail
- **THEN** the test process exits with code 1 and prints the failed test names and failure counts

### Requirement: Build scripts compile test binary
The system SHALL provide platform-specific build scripts that compile the test harness into an executable using gcc with the Fundamental Library's build conventions. For this chunk (no SIMD), the build SHALL not require AVX-512 flags.

#### Scenario: Windows amd64 build
- **WHEN** `build-windows-amd64.bat` is executed
- **THEN** it compiles all test source files plus `src/math/math_scalar.c` and `arch/math/windows-amd64/cpu_features.c` into `test.exe` using `gcc --std=c17 -Os -I ../../include`, strips the binary, and exits with code 0

#### Scenario: Linux amd64 build
- **WHEN** `build-linux-amd64.sh` is executed
- **THEN** it compiles all test source files plus `src/math/math_scalar.c` and `arch/math/linux-amd64/cpu_features.c` into `test` using `gcc --std=c17 -Os -I ../../include`, strips the binary, and exits with code 0

### Requirement: Consistency cross-validation
When a SIMD implementation is compiled alongside a scalar reference, the system SHALL verify that both produce identical results within the scalar's tolerance for randomly generated inputs.

#### Scenario: silu_f32 SIMD vs scalar consistency
- **WHEN** the consistency test generates 100 random input arrays of length 1024 and calls both `fun_math_silu_f32` (SIMD) and the scalar tail function on each
- **THEN** every output element matches within 1e-5 absolute tolerance

#### Scenario: rms_norm_f32 SIMD vs scalar consistency
- **WHEN** the consistency test generates 100 random input/weight arrays of length 4096 and calls both `fun_math_rms_norm_f32` (SIMD) and the scalar tail function on each
- **THEN** every output element matches within 1e-5 absolute tolerance

### Requirement: Harness self-test
The system SHALL include at least one deliberately incorrect golden value to verify the test harness correctly detects and reports failures.

#### Scenario: Harness detects deliberate failure
- **WHEN** the harness self-test runs against a golden case with an intentionally wrong expected value
- **THEN** it reports that case as failed, proving the harness can distinguish correct from incorrect results

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

