# Math Module Specification

## ADDED Requirements

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

## Constraints

- Implementation SHALL live in `arch/math/avx2/vector.c` (AVX2 + FMA).
- A scalar tail SHALL cover remainders, matching existing `_f32` kernels.
- No scalar fallback in `src/math/math_scalar.c` (consistent with existing
  `_f32` functions).
- The kernel SHALL NOT allocate memory or call transcendental functions.
- Model-specific RoPE constants (theta scale, ramp, mscale) SHALL remain in
  the demo, not the library.
