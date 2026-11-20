# Spec Delta

## MODIFIED Requirements

### Requirement: Data type enumeration
The system SHALL define a data type enumeration `FunMathDtype` with `F32`, `F16`, and `F64` members, all implemented by the generic dispatch. `F32` operates on float32 tensors; `F16` operates on `uint16_t` half-precision tensors converted through `fun_math_fp16_to_f32` and `fun_math_f32_to_fp16`; `F64` operates on `double` tensors computed in double precision.

#### Scenario: F32 is available
- **WHEN** a dispatch is invoked with `FUN_MATH_DTYPE_F32`
- **THEN** the operation executes on the float32 data

#### Scenario: Reserved dtypes are unsupported
- **WHEN** a dispatch is invoked with a `FunMathDtype` value that is not `F32`, `F16`, or `F64`
- **THEN** the operation is a no-op and the output is left unmodified

### Requirement: Operation enumeration
The system SHALL define an operation enumeration `FunMathOp` covering unary elementwise operations (sqrt, exp, log, sin, cos, tanh, sigmoid, silu), binary elementwise operations (add, sub, mul, div), and single-input reductions (sum, max, min, mean).

#### Scenario: Operation codes are enumerated
- **WHEN** a dispatch is invoked with any unary, binary, or reduction op code
- **THEN** the corresponding operation is applied to the input tensor(s)

### Requirement: Generic binary dispatch
The system SHALL provide `fun_math_binary(op, dtype, a, b, out)` that applies a binary elementwise operation (add, sub, mul, or div) to the two strided input tensors and writes the result to the output tensor, honoring each tensor's stride.

#### Scenario: Binary add
- **WHEN** `fun_math_binary(FUN_MATH_OP_ADD, FUN_MATH_DTYPE_F32, a, b, out)` is called
- **THEN** `out[i * out.stride]` equals `a[i * a.stride] + b[i * b.stride]` for every `i`

#### Scenario: Binary sub
- **WHEN** `fun_math_binary(FUN_MATH_OP_SUB, FUN_MATH_DTYPE_F32, a, b, out)` is called
- **THEN** `out[i * out.stride]` equals `a[i * a.stride] - b[i * b.stride]` for every `i`

#### Scenario: Binary mul
- **WHEN** `fun_math_binary(FUN_MATH_OP_MUL, FUN_MATH_DTYPE_F32, a, b, out)` is called
- **THEN** `out[i * out.stride]` equals `a[i * a.stride] * b[i * b.stride]` for every `i`

#### Scenario: Binary div
- **WHEN** `fun_math_binary(FUN_MATH_OP_DIV, FUN_MATH_DTYPE_F32, a, b, out)` is called
- **THEN** `out[i * out.stride]` equals `a[i * a.stride] / b[i * b.stride]` for every `i`

### Requirement: Generic reduction dispatch
The system SHALL provide `fun_math_reduce(op, dtype, in, out)` that reduces a strided input tensor to a single scalar written through `out`, a pointer to a scalar of the dtype. For `sum` it SHALL compute the sum, for `max` the maximum, for `min` the minimum, and for `mean` the arithmetic mean of the elements.

#### Scenario: Sum reduction
- **WHEN** `fun_math_reduce(FUN_MATH_OP_SUM, FUN_MATH_DTYPE_F32, in, out)` is called
- **THEN** `out` equals the sum of all `n` input elements within tolerance

#### Scenario: Max reduction
- **WHEN** `fun_math_reduce(FUN_MATH_OP_MAX, FUN_MATH_DTYPE_F32, in, out)` is called
- **THEN** `out` equals the maximum of all `n` input elements

#### Scenario: Min reduction
- **WHEN** `fun_math_reduce(FUN_MATH_OP_MIN, FUN_MATH_DTYPE_F32, in, out)` is called
- **THEN** `out` equals the minimum of all `n` input elements

#### Scenario: Mean reduction
- **WHEN** `fun_math_reduce(FUN_MATH_OP_MEAN, FUN_MATH_DTYPE_F32, in, out)` is called
- **THEN** `out` equals the sum of all `n` input elements divided by `n` within 1e-4 relative tolerance

#### Scenario: Zero-length reduction
- **WHEN** `fun_math_reduce` is called with a tensor whose `n` is 0
- **THEN** it writes 0.0 to `out` for `sum`, `max`, `min`, and `mean`

### Requirement: Structured kernel backend dispatch
The system SHALL route the structured kernels matrix-vector product, dot product, RMS norm, softmax, rotary rotation, swiglu, MXFP4 matrix-vector (including its strided variant), Q8 matrix-vector, Q8 dequantization, row-wise dot product, and weighted sum through the current backend. Each SHALL have a dispatch function that calls the backend's entry when set, and otherwise falls back to the corresponding built-in `fun_math_*` kernel.

#### Scenario: Default falls back to built-in kernel
- **WHEN** `fun_math_softmax_f32(x, n)` is called with the default CPU backend
- **THEN** the result matches `fun_math_softmax_f32_cpu(x, n)` exactly

#### Scenario: Custom backend intercepts
- **WHEN** a custom backend provides a softmax entry and is set as the current backend
- **THEN** `fun_math_softmax_f32` calls that entry instead of the built-in kernel

#### Scenario: Remaining kernels fall back to built-in
- **WHEN** each of `fun_math_swiglu_f32`, `fun_math_matrix_vector_mxfp4_f32`, `fun_math_matrix_vector_mxfp4_f32_strided`, `fun_math_q8_matrix_vector_f32`, `fun_math_q8_dequant_row_f32`, `fun_math_rows_dot_f32`, and `fun_math_weighted_sum_f32` is called with the default CPU backend
- **THEN** its result matches the corresponding `fun_math_*_cpu` built-in exactly

#### Scenario: Remaining kernels interceptable
- **WHEN** a custom backend provides one of the remaining structured entries and is set as the current backend
- **THEN** the dispatch function for that kernel calls the backend entry instead of the built-in

### Requirement: Portable elementwise binary and reduction kernels
The system SHALL provide portable `fun_math_add_f32`, `fun_math_sub_f32`, `fun_math_mul_f32`, `fun_math_div_f32`, `fun_math_sum_f32`, `fun_math_max_f32`, `fun_math_min_f32`, and `fun_math_mean_f32` kernels that operate over contiguous float32 arrays. These SHALL be the implementations the default CPU backend uses for the contiguous case of the generic binary and reduction dispatch.

#### Scenario: add_f32
- **WHEN** `fun_math_add_f32(a, b, out, n)` is called
- **THEN** `out[i]` equals `a[i] + b[i]` for every `i` in `[0, n)`

#### Scenario: sub_f32
- **WHEN** `fun_math_sub_f32(a, b, out, n)` is called
- **THEN** `out[i]` equals `a[i] - b[i]` for every `i` in `[0, n)`

#### Scenario: mul_f32
- **WHEN** `fun_math_mul_f32(a, b, out, n)` is called
- **THEN** `out[i]` equals `a[i] * b[i]` for every `i` in `[0, n)`

#### Scenario: div_f32
- **WHEN** `fun_math_div_f32(a, b, out, n)` is called
- **THEN** `out[i]` equals `a[i] / b[i]` for every `i` in `[0, n)`

#### Scenario: sum_f32
- **WHEN** `fun_math_sum_f32(x, n)` is called
- **THEN** it returns the sum of `x[0]` through `x[n-1]`

#### Scenario: max_f32
- **WHEN** `fun_math_max_f32(x, n)` is called
- **THEN** it returns the largest element of `x`

#### Scenario: min_f32
- **WHEN** `fun_math_min_f32(x, n)` is called
- **THEN** it returns the smallest element of `x`

#### Scenario: mean_f32
- **WHEN** `fun_math_mean_f32(x, n)` is called
- **THEN** it returns the sum of `x[0]` through `x[n-1]` divided by `n`

### Requirement: Backend interface is GPU-ready
The system SHALL define the backend as a set of function-pointer entries (unary, binary, reduction, and the listed structured kernels) that a future accelerator backend can implement and install with `fun_math_backend_set`. The backend struct SHALL be extensible so additional kernel entries can be appended without breaking existing backends. No GPU backend SHALL be implemented in this change.

#### Scenario: Backend contract is expressible
- **WHEN** a future backend implements the unary, binary, reduction, and structured entries
- **THEN** it can be installed with `fun_math_backend_set` and used by the dispatch call sites

#### Scenario: Extensible for additional kernels
- **WHEN** an additional kernel not yet in the backend (for example a new quantized format or a fused attention operator) is added in a future change
- **THEN** its entry is appended to the struct without breaking existing backends

## ADDED Requirements

### Requirement: Half-precision (F16) dispatch
The system SHALL support `FUN_MATH_DTYPE_F16` in the generic unary, binary, and reduction dispatch. Input tensors SHALL hold `uint16_t` half-precision elements converted to float32 with `fun_math_fp16_to_f32`, operated on in float32, and written back as `uint16_t` with `fun_math_f32_to_fp16`. The system SHALL provide `fun_math_f32_to_fp16` converting a float32 value to the nearest half-precision `uint16_t` encoding.

#### Scenario: F16 unary dispatch
- **WHEN** `fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F16, in, out)` is called on `uint16_t` tensors
- **THEN** every `out[i]` equals `fun_math_f32_to_fp16(fun_math_exp(fun_math_fp16_to_f32(in[i])))`

#### Scenario: F16 binary dispatch
- **WHEN** `fun_math_binary(FUN_MATH_OP_ADD, FUN_MATH_DTYPE_F16, a, b, out)` is called on `uint16_t` tensors
- **THEN** every `out[i]` equals the fp16 encoding of `fp16_to_f32(a[i]) + fp16_to_f32(b[i])`

#### Scenario: F16 reduction dispatch
- **WHEN** `fun_math_reduce(FUN_MATH_OP_SUM, FUN_MATH_DTYPE_F16, in, out)` is called on a `uint16_t` tensor
- **THEN** `out` is a `uint16_t` equal to the fp16 encoding of the float32 sum of the decoded elements

#### Scenario: f32 to f16 round trip
- **WHEN** `fun_math_f32_to_fp16(x)` is applied to a float32 value `x` and the result is converted back with `fun_math_fp16_to_f32`
- **THEN** the round-tripped value differs from `x` by no more than the half-precision rounding error (relative error below 1e-3)

### Requirement: Double-precision (F64) dispatch
The system SHALL support `FUN_MATH_DTYPE_F64` in the generic unary, binary, and reduction dispatch. Tensors SHALL hold `double` elements and every operation SHALL be computed in double precision, matching the double reference: unary transcendentals and `sum`/`mean` within 1e-9 relative error, and add/sub/mul/div/min/max exactly.

#### Scenario: F64 unary dispatch
- **WHEN** `fun_math_unary(FUN_MATH_OP_SQRT, FUN_MATH_DTYPE_F64, in, out)` is called on `double` tensors
- **THEN** every `out[i]` matches the double square root of `in[i]` within 1e-9 relative error

#### Scenario: F64 binary dispatch
- **WHEN** `fun_math_binary(FUN_MATH_OP_SUB, FUN_MATH_DTYPE_F64, a, b, out)` is called on `double` tensors
- **THEN** every `out[i]` equals `a[i] - b[i]` in double precision

#### Scenario: F64 reduction dispatch
- **WHEN** `fun_math_reduce(FUN_MATH_OP_MEAN, FUN_MATH_DTYPE_F64, in, out)` is called on a `double` tensor
- **THEN** `out` is a `double` equal to the mean of the elements within 1e-9 relative error

### Requirement: Contiguous unary dispatch uses batch kernels
The default CPU backend SHALL forward contiguous unary dispatch for `FUN_MATH_OP_EXP`, `FUN_MATH_OP_LOG`, `FUN_MATH_OP_SIN`, `FUN_MATH_OP_COS`, and `FUN_MATH_OP_SILU` to the corresponding batch kernels (`fun_math_exp_f32`, `fun_math_log_f32`, `fun_math_sin_f32`, `fun_math_cos_f32`, and `fun_math_silu_f32`). Unary ops without a batch kernel and strided tensors SHALL use the per-element scalar path.

#### Scenario: Contiguous unary matches the batch kernel
- **WHEN** `fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F32, in, out)` is called with a contiguous tensor
- **THEN** `out` equals `fun_math_exp_f32(in, out, n)` exactly

#### Scenario: Strided unary stays scalar
- **WHEN** `fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F32, in, out)` is called with a strided input tensor
- **THEN** every `out[i]` equals `fun_math_exp(in[i * in.stride])` within tolerance
