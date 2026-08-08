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

#### Scenario: Deferred vector functions
- **WHEN** `fun_math_silu_f32`, `fun_math_rms_norm_f32`, `fun_math_swiglu_f32`, or `fun_math_softmax_f32` is called
- **THEN** it returns 0.0 or returns immediately for void functions

#### Scenario: No deferred functions remain
- **WHEN** the math module is fully initialized after Chunk 3
- **THEN** all 12 declared functions have real implementations; no function returns a sentinel value

### Requirement: sin implementation
The system SHALL implement `fun_math_sin` using range reduction modulo 2π followed by a Taylor polynomial, achieving relative error below 1e-3 for inputs with absolute value ≤ 50.

#### Scenario: sin of values in valid range
- **WHEN** sin is called with inputs in [-50, 50]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: sin of zero
- **WHEN** sin is called with 0.0
- **THEN** it returns 0.0

#### Scenario: sin of NaN
- **WHEN** sin is called with NaN
- **THEN** it returns NaN

#### Scenario: sin odd symmetry
- **WHEN** sin is called with x
- **THEN** sin(-x) equals -sin(x) for any x in the valid domain

### Requirement: cos implementation
The system SHALL implement `fun_math_cos` using the identity `cos(x) = sin(x + π/2)`, achieving relative error below 1e-3 for inputs with absolute value ≤ 50.

#### Scenario: cos of values in valid range
- **WHEN** cos is called with inputs in [-50, 50]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: cos of zero
- **WHEN** cos is called with 0.0
- **THEN** it returns 1.0

#### Scenario: cos of NaN
- **WHEN** cos is called with NaN
- **THEN** it returns NaN

### Requirement: tanh implementation
The system SHALL implement `fun_math_tanh` using `fun_math_exp`, with saturation clamping at ±1 for |x| ≥ 10, achieving relative error below 1e-3 for all real inputs.

#### Scenario: tanh normal range
- **WHEN** tanh is called with inputs in [-10, 10]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: tanh saturation
- **WHEN** tanh is called with inputs ≥ 10
- **THEN** it returns 1.0
- **WHEN** tanh is called with inputs ≤ -10
- **THEN** it returns -1.0

#### Scenario: tanh special values
- **WHEN** tanh is called with 0.0
- **THEN** it returns 0.0
- **WHEN** tanh is called with +inf
- **THEN** it returns 1.0
- **WHEN** tanh is called with -inf
- **THEN** it returns -1.0
- **WHEN** tanh is called with NaN
- **THEN** it returns NaN

### Requirement: sigmoid implementation
The system SHALL implement `fun_math_sigmoid` using `fun_math_exp`, with saturation clamping at 0 and 1 for |x| ≥ 20, achieving relative error below 1e-3 for all real inputs.

#### Scenario: sigmoid normal range
- **WHEN** sigmoid is called with inputs in [-20, 20]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: sigmoid saturation
- **WHEN** sigmoid is called with inputs ≥ 20
- **THEN** it returns 1.0
- **WHEN** sigmoid is called with inputs ≤ -20
- **THEN** it returns 0.0

#### Scenario: sigmoid special values
- **WHEN** sigmoid is called with 0.0
- **THEN** it returns 0.5
- **WHEN** sigmoid is called with +inf
- **THEN** it returns 1.0
- **WHEN** sigmoid is called with -inf
- **THEN** it returns 0.0
- **WHEN** sigmoid is called with NaN
- **THEN** it returns NaN

### Requirement: silu implementation
The system SHALL implement `fun_math_silu` using `fun_math_sigmoid`, achieving relative error below 1e-3 for all real inputs.

#### Scenario: silu normal range
- **WHEN** silu is called with inputs in [-20, 20]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: silu saturation
- **WHEN** silu is called with inputs ≥ 20
- **THEN** it returns x (approximately the identity function)
- **WHEN** silu is called with inputs ≤ -20
- **THEN** it returns 0.0

#### Scenario: silu special values
- **WHEN** silu is called with 0.0
- **THEN** it returns 0.0
- **WHEN** silu is called with +inf
- **THEN** it returns +inf
- **WHEN** silu is called with -inf
- **THEN** it returns 0.0
- **WHEN** silu is called with NaN
- **THEN** it returns NaN

### Requirement: silu_f32 AVX2 implementation
The system SHALL implement `fun_math_silu_f32` using AVX2 SIMD when available, applying element-wise SiLU activation over float32 arrays with aligned loads and stores.

#### Scenario: silu_f32 accuracy
- **WHEN** silu_f32 is called with arrays of length 1024 against precomputed golden values
- **THEN** every element is within 1e-4 absolute or 1e-3 relative tolerance of the expected value

#### Scenario: silu_f32 zero-length
- **WHEN** silu_f32 is called with n=0
- **THEN** it returns immediately without accessing input or output arrays

#### Scenario: silu_f32 AVX2 vs scalar consistency
- **WHEN** silu_f32 is called on AVX2-capable hardware and compared against per-element scalar silu
- **THEN** every element matches within 1e-5 absolute tolerance

### Requirement: rms_norm_f32 AVX2 implementation
The system SHALL implement `fun_math_rms_norm_f32` using AVX2 SIMD when available, computing RMS normalization over float32 arrays with aligned loads and stores.

#### Scenario: rms_norm_f32 accuracy
- **WHEN** rms_norm_f32 is called with arrays of length 4096 against precomputed golden values
- **THEN** every element is within 1e-4 absolute or 1e-3 relative tolerance of the expected value

#### Scenario: rms_norm_f32 zero-length
- **WHEN** rms_norm_f32 is called with n=0
- **THEN** it returns immediately without accessing input or output arrays

#### Scenario: rms_norm_f32 AVX2 vs scalar consistency
- **WHEN** rms_norm_f32 is called on AVX2-capable hardware and compared against per-element scalar reference
- **THEN** every element matches within 5e-4 absolute or 5e-3 relative tolerance

### Requirement: swiglu_f32 AVX2 implementation
The system SHALL implement `fun_math_swiglu_f32` using AVX2 SIMD when available, computing gate·SiLU(gate)·up over float32 arrays with aligned loads and stores.

#### Scenario: swiglu_f32 accuracy
- **WHEN** swiglu_f32 is called with gate/up arrays of length 1024 against precomputed golden values
- **THEN** every element is within 1e-4 absolute or 1e-3 relative tolerance of the expected value

#### Scenario: swiglu_f32 zero-length
- **WHEN** swiglu_f32 is called with n=0
- **THEN** it returns immediately without accessing input or output arrays

### Requirement: softmax_f32 AVX2 implementation
The system SHALL implement `fun_math_softmax_f32` using AVX2 SIMD when available, computing in-place softmax over float32 arrays with aligned loads and stores.

#### Scenario: softmax_f32 accuracy
- **WHEN** softmax_f32 is called with arrays of length 32 against precomputed golden values
- **THEN** every element is within 1e-4 absolute or 1e-3 relative tolerance of the expected value, and the output sum is within 1e-4 of 1.0

#### Scenario: softmax_f32 zero-length
- **WHEN** softmax_f32 is called with n=0
- **THEN** it returns immediately without accessing the input array

### Requirement: Vector function dispatch
The system SHALL dispatch vector function calls to the best available implementation at runtime based on CPU feature detection.

#### Scenario: AVX2 dispatch
- **WHEN** the CPU supports AVX2 and `fun_math_init` has been called
- **THEN** all four vector functions execute AVX2-optimized code paths

#### Scenario: Scalar fallback
- **WHEN** the CPU does not support AVX2
- **THEN** all four vector functions execute scalar fallback implementations

### Requirement: Aligned-only AVX2 API
The AVX2 vector implementations SHALL require 32-byte aligned buffers and n%8==0. Callers violating these preconditions receive undefined behavior.

#### Scenario: AVX2 path assumes alignment
- **WHEN** the AVX2 code path is active
- **THEN** aligned load and store instructions are used with no remainder loop or safety checks

### Requirement: ISA units provide vector function implementations
Vector function implementations SHALL be provided by ISA-specific compilation units under `arch/math/<isa>/`. The compilation unit provides strong definitions of the public `fun_math_*` vector functions, selected at link time by which `arch/math/<isa>/` directory is included in the build. No runtime dispatch layer and no function-pointer fallback SHALL exist.

#### Scenario: AVX2 unit linked
- **WHEN** `arch/math/avx2/vector.c` is compiled and linked into the binary
- **THEN** all four vector functions (silu_f32, swiglu_f32, rms_norm_f32, softmax_f32) execute AVX2-optimized code paths without runtime branching

#### Scenario: non-AVX2 build
- **WHEN** the AVX2 unit is not part of the build
- **THEN** the build must provide another ISA unit or fail to link; no implicit scalar fallback exists

### Requirement: Vector-safe vector API
The AVX2 vector implementations SHALL accept unaligned buffers and any `n`. The vectorized main loop SHALL use unaligned loads/stores (`_mm256_loadu_ps`/`_mm256_storeu_ps`), and a scalar tail loop SHALL process the remaining `n % 8` elements.

#### Scenario: unaligned buffers
- **WHEN** the AVX2 code path processes buffers not aligned to 32 bytes
- **THEN** unaligned load/store instructions are used and results remain correct

#### Scenario: tail remainder
- **WHEN** `n` is not a multiple of 8
- **THEN** the remaining `n % 8` elements are processed by the scalar tail loop without out-of-bounds access

#### Scenario: vectorized correctness
- **WHEN** AVX2 results are compared with scalar `fun_math_*` results
- **THEN** each element matches within tolerance (1e-4 relative, 1e-3 absolute) and golden expected values

### Requirement: Public API vector additions
The system SHALL extend `include/fundamental/math/math.h` with declarations for matrix-vector multiplication, dot product, and batched transcendental functions, following the existing `fun_math_*_f32` naming convention.

#### Scenario: Header declares matrix-vector and dot functions
- **WHEN** the header is included
- **THEN** the following functions are declared: `fun_math_matrix_vector_f32(const float *w, const float *x, const float *bias, float *out, size_t rows, size_t cols)` and `fun_math_dot_f32(const float *a, const float *b, size_t n)`

#### Scenario: Header declares batched transcendentals
- **WHEN** the header is included
- **THEN** the following functions are declared: `fun_math_exp_f32(const float *x, float *out, size_t n)`, `fun_math_log_f32(const float *x, float *out, size_t n)`, `fun_math_sin_f32(const float *x, float *out, size_t n)`, and `fun_math_cos_f32(const float *x, float *out, size_t n)`

### Requirement: matrix_vector_f32 implementation
The system SHALL implement `fun_math_matrix_vector_f32` so that for each row `r` in `[0, rows)`, `out[r]` equals `bias[r]` (when `bias` is not NULL, otherwise 0) plus the dot product of row `r` of `w` (elements `w[r*cols + c]`) with `x[c]`, within tolerance of a scalar reference computation. The implementation SHALL process the vectorized main loop with SIMD and handle the remaining `cols % 8` elements with a scalar tail loop.

#### Scenario: matrix-vector accuracy
- **WHEN** matrix_vector_f32 is called with a matrix of rows x cols against a naive scalar reference
- **THEN** every output element matches the reference within 1e-3 relative or 1e-4 absolute tolerance

#### Scenario: matrix-vector without bias
- **WHEN** matrix_vector_f32 is called with `bias = NULL`
- **THEN** each output element equals the unbiased row dot product

#### Scenario: matrix-vector with bias
- **WHEN** matrix_vector_f32 is called with a non-NULL bias array
- **THEN** each output element equals the row dot product plus the corresponding bias element

#### Scenario: matrix-vector non-multiple-of-8 columns
- **WHEN** matrix_vector_f32 is called with `cols` not divisible by 8
- **THEN** all output elements are correct (tail elements processed without out-of-bounds access)

### Requirement: dot_f32 implementation
The system SHALL implement `fun_math_dot_f32` so that it returns the sum of `a[i] * b[i]` for `i` in `[0, n)` within tolerance of a scalar reference, using a SIMD main loop with a scalar tail for `n % 8`.

#### Scenario: dot product accuracy
- **WHEN** dot_f32 is called with two arrays of length n against a naive scalar reference
- **THEN** the result matches the reference within 1e-3 relative or 1e-4 absolute tolerance

#### Scenario: dot product zero length
- **WHEN** dot_f32 is called with `n = 0`
- **THEN** it returns 0.0

### Requirement: exp_f32 implementation
The system SHALL implement `fun_math_exp_f32` to compute `exp(x[i])` for each element with SIMD, a scalar tail for `n % 8`, and saturation semantics identical to scalar `fun_math_exp`: NaN in → NaN out, `+inf` in → `+inf` out, `-inf` in → 0 out, input ≥ 88.7228 → `+inf`, input ≤ -87.3365 → 0.

#### Scenario: exp_f32 accuracy
- **WHEN** exp_f32 is called with arrays spanning [-88, 88] against reference exp values
- **THEN** every output element is within 1e-3 relative or 1e-4 absolute tolerance

#### Scenario: exp_f32 negative infinity input
- **WHEN** exp_f32 is called with an element equal to -inf
- **THEN** the corresponding output element is 0.0

#### Scenario: exp_f32 overflow saturation
- **WHEN** exp_f32 is called with an element of 100.0
- **THEN** the corresponding output element is +inf

#### Scenario: exp_f32 underflow saturation
- **WHEN** exp_f32 is called with an element of -100.0
- **THEN** the corresponding output element is 0.0

#### Scenario: exp_f32 NaN propagation
- **WHEN** exp_f32 is called with an element equal to NaN
- **THEN** the corresponding output element is NaN

### Requirement: log_f32 implementation
The system SHALL implement `fun_math_log_f32` to compute `log(x[i])` for each element with SIMD, a scalar tail for `n % 8`, and edge semantics identical to scalar `fun_math_log`: NaN in → NaN out, `+inf` in → `+inf` out, 0 or -0 in → `-inf` out, negative finite in → NaN out.

#### Scenario: log_f32 accuracy
- **WHEN** log_f32 is called with arrays spanning [0.001, 1e6] against reference log values
- **THEN** every output element is within 1e-3 relative or 1e-4 absolute tolerance

#### Scenario: log_f32 zero input
- **WHEN** log_f32 is called with an element equal to 0.0
- **THEN** the corresponding output element is -inf

#### Scenario: log_f32 negative input
- **WHEN** log_f32 is called with a negative finite element
- **THEN** the corresponding output element is NaN

#### Scenario: log_f32 infinity input
- **WHEN** log_f32 is called with an element equal to +inf
- **THEN** the corresponding output element is +inf

### Requirement: sin_f32 and cos_f32 implementations
The system SHALL implement `fun_math_sin_f32` and `fun_math_cos_f32` to compute `sin(x[i])` and `cos(x[i])` for each element with SIMD and a scalar tail for `n % 8`, achieving the same accuracy domain guarantee as the scalar functions (|x| ≤ 50, relative error below 1e-3) and propagating NaN inputs.

#### Scenario: sin_f32 accuracy
- **WHEN** sin_f32 is called with arrays spanning [-50, 50] against reference sin values
- **THEN** every output element is within 1e-3 relative or 1e-4 absolute tolerance

#### Scenario: cos_f32 accuracy
- **WHEN** cos_f32 is called with arrays spanning [-50, 50] against reference cos values
- **THEN** every output element is within 1e-3 relative or 1e-4 absolute tolerance

#### Scenario: sin/cos zero length
- **WHEN** sin_f32 or cos_f32 is called with `n = 0`
- **THEN** it returns immediately without accessing input or output arrays

### Requirement: Vector function edge cases
The new vector functions SHALL handle special values and degenerate sizes consistently with the existing vector functions.

#### Scenario: Zero-length vector calls
- **WHEN** any new vector function (matrix_vector_f32 with cols=0, dot_f32 with n=0, exp_f32, log_f32, sin_f32, cos_f32) is called with size 0
- **THEN** it returns immediately without accessing input or output arrays

#### Scenario: SIMD vs scalar consistency
- **WHEN** each new vector function is called on the SIMD implementation and compared against per-element scalar reference results
- **THEN** every output element matches within 1e-4 absolute or 1e-3 relative tolerance

### Requirement: Rotary Rotation
The system SHALL provide `fun_math_rotary_f32` that applies the rotation
`out[j] = x[j]*cosv[j] - x[j+half]*sinv[j]` and
`out[j+half] = x[j]*sinv[j] + x[j+half]*cosv[j]` for every pair `(j, j+half)`
in each of `n_heads` contiguous head blocks of length `2*half`.

#### Scenario: Rotate a single head
- **GIVEN** x of 2*half floats, cosv and sinv of half floats
- **WHEN** fun_math_rotary_f32 is called with n_heads=1
- **THEN** first half of out matches x[j]*cosv[j] - x[j+half]*sinv[j]
- **AND** second half of out matches x[j]*sinv[j] + x[j+half]*cosv[j]

#### Scenario: Rotate multiple heads with shared tables
- **GIVEN** x with n_heads contiguous blocks of 2*half floats
- **WHEN** fun_math_rotary_f32 is called with n_heads>1 and shared cosv/sinv
- **THEN** every head block is rotated identically
- **AND** no element crosses a head boundary (pairs stay within a block)

#### Scenario: In-place rotation
- **WHEN** fun_math_rotary_f32 is called with out aliasing x
- **THEN** the result matches the out-of-place result within tolerance

#### Scenario: Half values not multiple of 8
- **GIVEN** half such that half % 8 != 0 (e.g. 7, 9, 31, 33)
- **WHEN** fun_math_rotary_f32 is called
- **THEN** all elements match the scalar rotation formula
- **AND** the scalar tail covers the remaining lanes

#### Scenario: Edge sizes
- **GIVEN** n_heads=0 or half=0
- **WHEN** fun_math_rotary_f32 is called
- **THEN** the function returns without accessing memory

### Requirement: Rotary Numerical Accuracy
The system SHALL match the scalar reference rotation within tolerance.

#### Scenario: Golden value match
- **GIVEN** golden cases generated from the scalar reference formula
- **WHEN** fun_math_rotary_f32 processes each case
- **THEN** every output element is within 1e-4 absolute / 1e-3 relative
  tolerance of the golden expected value

### Requirement: Demo RoPE Reuse
The gpt-demo SHALL use the primitive for its RoPE rotation.

#### Scenario: Demo RoPE uses the primitive
- **GIVEN** a loaded model
- **WHEN** model_forward runs RoPE on a position
- **THEN** the theta series is computed once from the precomputed rope_pre
  table (not per head)
- **AND** cos/sin are computed once and shared by all heads
- **AND** the rotation uses fun_math_rotary_f32 for q and k

#### Scenario: Demo output preserved
- **GIVEN** the same prompt and model
- **WHEN** generation runs after the refactor
- **THEN** the greedy argmax token sequence is unchanged from before

