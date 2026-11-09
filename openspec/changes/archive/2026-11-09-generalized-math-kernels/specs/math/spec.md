# Spec Delta

## MODIFIED Requirements

### Requirement: ISA units provide vector function implementations
Vector function implementations SHALL be provided by ISA-specific compilation units under `arch/math/<isa>/` (`scalar`, `avx2`, `avx512`). Each unit defines the public `fun_math_*` vector functions for one ISA. The implementation that executes is selected at link time by which units are compiled and linked into the binary. There is no runtime dispatch layer and no function-pointer fallback for choosing among CPU ISAs; CPU feature detection is never used to select an implementation.

The scalar unit SHALL provide strong definitions used when no SIMD unit is linked. The AVX2 unit SHALL define its kernels with weak linkage (`__attribute__((weak))`) so a build may link the AVX-512 unit alongside it and resolve each function to the AVX-512 implementation. Backend-level selection (CPU vs. accelerator) is a separate, opt-in mechanism governed by the "Backend registry" requirement and SHALL NOT be used to choose among CPU ISA implementations.

#### Scenario: AVX-512 build resolves to AVX-512
- **WHEN** `arch/math/avx512/vector.c` and `arch/math/avx2/vector.c` are both compiled and linked into the binary
- **THEN** the weak AVX2 definitions defer to the AVX-512 definitions at link time and the vector functions execute AVX-512 code paths with no runtime branching

#### Scenario: AVX2 unit linked alone
- **WHEN** only `arch/math/avx2/vector.c` (with its MXFP4 unit) is linked
- **THEN** the vector functions execute AVX2-optimized code paths without runtime branching

#### Scenario: Scalar-only build
- **WHEN** only `arch/math/scalar/vector.c` is linked
- **THEN** the vector functions execute the portable scalar implementations

#### Scenario: no runtime dispatch
- **WHEN** a built-in `fun_math_*` vector function is called
- **THEN** the executed implementation is fixed at link time; the call never branches on CPUID results or a function-pointer table to choose among CPU ISA implementations

### Requirement: Public API vector additions
The system SHALL extend `include/fundamental/math/math.h` with declarations for matrix-vector multiplication, dot product, batched transcendental functions, and elementwise binary and reduction kernels, following the existing `fun_math_*_f32` naming convention.

#### Scenario: Header declares matrix-vector and dot functions
- **WHEN** the header is included
- **THEN** the following functions are declared: `fun_math_matrix_vector_f32(const float *w, const float *x, const float *bias, float *out, size_t rows, size_t cols)` and `fun_math_dot_f32(const float *a, const float *b, size_t n)`

#### Scenario: Header declares batched transcendentals
- **WHEN** the header is included
- **THEN** the following functions are declared: `fun_math_exp_f32(const float *x, float *out, size_t n)`, `fun_math_log_f32(const float *x, float *out, size_t n)`, `fun_math_sin_f32(const float *x, float *out, size_t n)`, and `fun_math_cos_f32(const float *x, float *out, size_t n)`

#### Scenario: Header declares elementwise binary and reduction kernels
- **WHEN** the header is included
- **THEN** the following functions are declared: `fun_math_add_f32(const float *a, const float *b, float *out, size_t n)`, `fun_math_mul_f32(const float *a, const float *b, float *out, size_t n)`, `fun_math_sum_f32(const float *x, size_t n)`, and `fun_math_max_f32(const float *x, size_t n)`

## ADDED Requirements

### Requirement: Backend registry
The system SHALL provide a backend registry for the math module with a getter and setter. The default backend SHALL be the built-in CPU backend that forwards to the built-in `fun_math_*` kernels (including the elementwise binary and reduction kernels added by this change). Setting a custom backend SHALL replace the backend used by subsequent dispatches; passing NULL SHALL restore the default CPU backend.

#### Scenario: Default backend is the CPU
- **WHEN** `fun_math_backend_get()` is called before any backend is set
- **THEN** it returns the built-in CPU backend

#### Scenario: Set and restore a custom backend
- **WHEN** a caller calls `fun_math_backend_set(custom)`
- **THEN** `fun_math_backend_get()` returns `custom`
- **WHEN** the caller subsequently calls `fun_math_backend_set(NULL)`
- **THEN** `fun_math_backend_get()` returns the default CPU backend

#### Scenario: Backend set outside concurrent dispatch
- **WHEN** `fun_math_backend_set` is called during single-threaded startup before any dispatch
- **THEN** the registry is a process-global slot and subsequent dispatches use the selected backend

### Requirement: Data type enumeration
The system SHALL define a data type enumeration `FunMathDtype` with an `F32` member that is implemented, and `F16` and `F64` members that are reserved for future backends.

#### Scenario: F32 is available
- **WHEN** a dispatch is invoked with `FUN_MATH_DTYPE_F32`
- **THEN** the operation executes on the float32 data

#### Scenario: Reserved dtypes are unsupported
- **WHEN** a dispatch is invoked with `FUN_MATH_DTYPE_F16` or `FUN_MATH_DTYPE_F64`
- **THEN** the operation is a no-op and the output is left unmodified

### Requirement: Operation enumeration
The system SHALL define an operation enumeration `FunMathOp` covering unary elementwise operations (sqrt, exp, log, sin, cos, tanh, sigmoid, silu), binary elementwise operations (add, mul), and single-input reductions (sum, max).

#### Scenario: Operation codes are enumerated
- **WHEN** a dispatch is invoked with any unary, binary, or reduction op code
- **THEN** the corresponding operation is applied to the input tensor(s)

### Requirement: Strided tensor descriptor
The system SHALL define a tensor descriptor `FunMathTensor` carrying a data pointer, an element count `n`, and a stride in elements. A stride of 1 SHALL denote a contiguous tensor.

#### Scenario: Contiguous tensor
- **WHEN** a dispatch is invoked with a tensor whose stride is 1 and count is `n`
- **THEN** the operation reads or writes `n` contiguous elements starting at `data`

#### Scenario: Strided tensor
- **WHEN** a dispatch is invoked with a tensor whose stride is `s`
- **THEN** the operation accesses elements `data[i * s]` for `i` in `[0, n)`

### Requirement: Generic unary dispatch
The system SHALL provide `fun_math_unary(op, dtype, in, out)` that applies a unary elementwise operation to each element of the strided input tensor and writes the result to the output tensor, which is itself a `FunMathTensor` whose stride is honored. For `F32`, output SHALL match the corresponding scalar function within tolerance, and NaN inputs SHALL produce NaN outputs.

#### Scenario: Unary exp over a contiguous tensor
- **WHEN** `fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F32, in, out)` is called with a contiguous input and output
- **THEN** `out[i]` equals `exp(in[i])` within 1e-4 relative or 1e-5 absolute tolerance for every `i`

#### Scenario: Unary over a strided output tensor
- **WHEN** `fun_math_unary` is called with an output tensor whose stride is `s`
- **THEN** the result is written at `out[i * s]` for every `i`

#### Scenario: All unary ops match their scalar reference
- **WHEN** `fun_math_unary(op, FUN_MATH_DTYPE_F32, in, out)` is called for each op in sqrt, exp, log, sin, cos, tanh, sigmoid, and silu
- **THEN** `out[i]` matches the corresponding scalar `fun_math_*` function within 1e-4 relative or 1e-5 absolute tolerance for every `i`

#### Scenario: NaN propagation
- **WHEN** `fun_math_unary` is called with a NaN element
- **THEN** the corresponding output element is NaN

### Requirement: Generic binary dispatch
The system SHALL provide `fun_math_binary(op, dtype, a, b, out)` that applies a binary elementwise operation (add or mul) to the two strided input tensors and writes the result to the output tensor, honoring each tensor's stride.

#### Scenario: Binary add
- **WHEN** `fun_math_binary(FUN_MATH_OP_ADD, FUN_MATH_DTYPE_F32, a, b, out)` is called
- **THEN** `out[i * out.stride]` equals `a[i * a.stride] + b[i * b.stride]` for every `i`

#### Scenario: Binary mul
- **WHEN** `fun_math_binary(FUN_MATH_OP_MUL, FUN_MATH_DTYPE_F32, a, b, out)` is called
- **THEN** `out[i * out.stride]` equals `a[i * a.stride] * b[i * b.stride]` for every `i`

### Requirement: Generic reduction dispatch
The system SHALL provide `fun_math_reduce(op, dtype, in, out)` that reduces a strided input tensor to a single scalar written through `out`, a pointer to a scalar of the dtype. For `sum` it SHALL compute the sum of the elements, and for `max` it SHALL compute the maximum element.

#### Scenario: Sum reduction
- **WHEN** `fun_math_reduce(FUN_MATH_OP_SUM, FUN_MATH_DTYPE_F32, in, out)` is called
- **THEN** `out` equals the sum of all `n` input elements within tolerance

#### Scenario: Max reduction
- **WHEN** `fun_math_reduce(FUN_MATH_OP_MAX, FUN_MATH_DTYPE_F32, in, out)` is called
- **THEN** `out` equals the maximum of all `n` input elements

#### Scenario: Zero-length reduction
- **WHEN** `fun_math_reduce` is called with a tensor whose `n` is 0
- **THEN** it writes 0.0 to `out` for both `sum` and `max`

### Requirement: Structured kernel backend dispatch
The system SHALL route the structured kernels matrix-vector product, dot product, RMS norm, softmax, and rotary rotation through the current backend. Each SHALL have a dispatch function that calls the backend's entry when set, and otherwise falls back to the corresponding built-in `fun_math_*` kernel.

#### Scenario: Default falls back to built-in kernel
- **WHEN** `fun_math_softmax_f32_dispatch(x, n)` is called with the default CPU backend
- **THEN** the result matches `fun_math_softmax_f32(x, n)` exactly

#### Scenario: Custom backend intercepts
- **WHEN** a custom backend provides a softmax entry and is set as the current backend
- **THEN** `fun_math_softmax_f32_dispatch` calls that entry instead of the built-in kernel

### Requirement: Portable elementwise binary and reduction kernels
The system SHALL provide portable `fun_math_add_f32`, `fun_math_mul_f32`, `fun_math_sum_f32`, and `fun_math_max_f32` kernels that operate over contiguous float32 arrays. These SHALL be the implementations the default CPU backend uses for the contiguous case of the generic binary and reduction dispatch.

#### Scenario: add_f32
- **WHEN** `fun_math_add_f32(a, b, out, n)` is called
- **THEN** `out[i]` equals `a[i] + b[i]` for every `i` in `[0, n)`

#### Scenario: mul_f32
- **WHEN** `fun_math_mul_f32(a, b, out, n)` is called
- **THEN** `out[i]` equals `a[i] * b[i]` for every `i` in `[0, n)`

#### Scenario: sum_f32
- **WHEN** `fun_math_sum_f32(x, n)` is called
- **THEN** it returns the sum of `x[0]` through `x[n-1]`

#### Scenario: max_f32
- **WHEN** `fun_math_max_f32(x, n)` is called
- **THEN** it returns the largest element of `x`

### Requirement: Backend interface is GPU-ready
The system SHALL define the backend as a set of function-pointer entries (unary, binary, reduction, and the listed structured kernels) that a future accelerator backend can implement and install with `fun_math_backend_set`. The backend struct SHALL be extensible so additional kernel entries can be appended without breaking existing backends. No GPU backend SHALL be implemented in this change.

#### Scenario: Backend contract is expressible
- **WHEN** a future backend implements the unary, binary, reduction, and structured entries
- **THEN** it can be installed with `fun_math_backend_set` and used by the dispatch call sites

#### Scenario: Extensible for additional kernels
- **WHEN** additional kernels (for example swiglu or quantized matrix-vector) are added to the backend in a future change
- **THEN** their entries are appended to the struct without breaking existing backends
