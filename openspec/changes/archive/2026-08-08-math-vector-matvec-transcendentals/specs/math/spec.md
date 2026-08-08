## ADDED Requirements

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
