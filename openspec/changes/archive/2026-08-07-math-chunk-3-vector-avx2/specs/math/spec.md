## ADDED Requirements

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

## MODIFIED Requirements

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
