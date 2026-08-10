## ADDED Requirements

### Requirement: Row-wise dot product primitive
The system SHALL provide `fun_math_rows_dot_f32` that computes, for each row
`t` in `[0, n_rows)`, the dot product of a shared query vector `q` (length
`row_len`) with the strided row `x + t*row_stride`, multiplies the result by a
caller-supplied `scale`, and stores it at `out[t]`. The implementation SHALL
process the main loop with SIMD and handle the remaining `row_len % 8`
elements with a scalar tail without out-of-bounds access.

#### Scenario: Single row
- **WHEN** `fun_math_rows_dot_f32` is called with `n_rows=1`
- **THEN** `out[0]` equals `scale * dot(q, x)` within tolerance

#### Scenario: Multiple strided rows
- **WHEN** `fun_math_rows_dot_f32` is called with `n_rows > 1` and
  `row_stride >= row_len`
- **THEN** every `out[t]` matches the scalar reference `scale * dot(q, x +
  t*row_stride)` within 1e-4 absolute or 1e-3 relative tolerance

#### Scenario: Non-multiple-of-8 row length
- **WHEN** `row_len % 8 != 0`
- **THEN** the scalar tail covers the remaining lanes and every output element
  is correct

#### Scenario: Scale folding
- **WHEN** a non-1.0 `scale` is passed
- **THEN** every output element is scaled by that value

#### Scenario: Zero rows
- **WHEN** `n_rows == 0` or `row_len == 0`
- **THEN** the function returns without accessing input or output memory

### Requirement: Weighted sum accumulation primitive
The system SHALL provide `fun_math_weighted_sum_f32` that computes
`out[d] = sum_{t<n_rows} wgt[t] * x[t*row_stride + d]` for `d` in
`[0, row_len)`, overwriting (compute-and-store) the caller's `out` buffer. The
implementation SHALL process the main loop with SIMD and handle the remaining
`row_len % 8` elements with a scalar tail.

#### Scenario: Weighted column sum
- **WHEN** `fun_math_weighted_sum_f32` is called with weights, a strided row
  matrix, and an output buffer
- **THEN** every `out[d]` matches the scalar reference sum within 1e-4 absolute
  or 1e-3 relative tolerance

#### Scenario: Compute-and-store semantics
- **WHEN** the output buffer holds pre-existing values
- **THEN** the result overwrites those values (no accumulation on top of them)

#### Scenario: Non-multiple-of-8 row length
- **WHEN** `row_len % 8 != 0`
- **THEN** the scalar tail covers the remaining lanes and every output element
  is correct

#### Scenario: Zero rows
- **WHEN** `n_rows == 0` or `row_len == 0`
- **THEN** the function returns without accessing input memory and leaves
  `out` untouched

### Requirement: Demo attention vectorization
The gpt-demo SHALL compute attention scores with the row-wise dot primitive
and accumulate the value-weighted sums with the weighted-sum primitive, and
SHALL normalize attention weights with the existing softmax primitive by
appending the sink logit as an extra softmax element.

#### Scenario: Score computation uses the primitive
- **WHEN** model_forward computes attention scores for a head
- **THEN** each score equals `dot(q_h, k_t) * inv_sqrt_hd` computed by
  `fun_math_rows_dot_f32` in one call per head

#### Scenario: Value accumulation uses the primitive
- **WHEN** model_forward accumulates the weighted value vectors for a head
- **THEN** the accumulation is performed by `fun_math_weighted_sum_f32` in one
  call per head

#### Scenario: Softmax includes the sink
- **WHEN** model_forward normalizes attention weights for a head
- **THEN** the sink logit is included as an extra element of the softmax input
  and the normalized weights match the scalar reference softmax within 1e-4
  absolute or 1e-3 relative tolerance

#### Scenario: Greedy output preserved
- **WHEN** generation runs after the refactor with the same prompt and model
- **THEN** the greedy argmax token sequence is unchanged from the pre-refactor
  baseline

### Requirement: Fused MXFP4 matrix-vector primitive
The system SHALL provide `fun_math_matrix_vector_mxfp4_f32` that computes
`out[r] = sum_{d<row_len} mxfp4(w[r], d) * x[d] + b[r]` for each row `r` in
`[0, n_rows)`, reading the weight matrix directly from GGUF MXFP4 blocks
(32 elements per 17-byte block: one E8M0 scale byte followed by 16 nibble
bytes; E2M1 kvalue table `{0, 0.5, 1, 1.5, 2, 3, 4, 6, -0, -0.5, -1, -1.5, -2,
-3, -4, -6}`; scale `0` decodes to `0.0f`, otherwise the scale byte shifted
into the exponent field of an f32). The implementation SHALL require
`row_len % 32 == 0` (block alignment) and SHALL process the main loop with SIMD
using the ×2-scaled integer lookup table with an exponent-shifted scale.

#### Scenario: Row matches scalar reference
- **WHEN** `fun_math_matrix_vector_mxfp4_f32` is called with a block-aligned
  weight matrix, an activation vector, and a bias vector
- **THEN** every `out[r]` matches the scalar reference (dequantize with the
  E2M1/E8M0 math, then dot) within 1e-4 absolute or 1e-3 relative tolerance

#### Scenario: Zero scale blocks
- **WHEN** a block's scale byte is 0
- **THEN** that block contributes zero regardless of its nibble values

#### Scenario: Negative and fractional kvalues
- **WHEN** nibbles encode negative or fractional E2M1 kvalues (0.5, 1.5, 3, 6
  and their negatives)
- **THEN** the row result matches the scalar reference within tolerance

#### Scenario: Bias applied
- **WHEN** a non-null bias vector is passed
- **THEN** every `out[r]` includes `b[r]` added after the weighted sum

#### Scenario: Zero rows
- **WHEN** `n_rows == 0`
- **THEN** the function returns without accessing input or output memory
