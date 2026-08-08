## Context

See proposal.md — Why. The math module's vector functions (`fun_math_rms_norm_f32`, `fun_math_silu_f32`, `fun_math_swiglu_f32`, `fun_math_softmax_f32`) are provided exclusively by the AVX2 ISA unit `arch/math/avx2/vector.c`, selected at link time (no runtime dispatch — see archived change `remove-math-dispatch`). That file already contains a private static `_mm256_exp_ps` (Pade approximant + range reduction, identical coefficients to scalar `fun_math_exp`) that the new public functions will build on. The gpt-demo currently hand-rolls `mat_vec_f32` and `mat_vec_dot32` with raw intrinsics in `model.c` and calls scalar `fun_math_exp/sin/cos` per element in RoPE, attention softmax, and MoE gating.

## Goals / Non-Goals

**Goals:**
- Public, module-owned matrix-vector and dot product functions usable by any consumer.
- Public batched (vectorized) `exp_f32`, `log_f32`, `sin_f32`, `cos_f32`.
- gpt-demo consumes the new API; no raw intrinsics remain in `model.c`.
- Edge semantics of the batch functions match the existing scalar functions (masked softmax with `-inf` must keep working).

**Non-Goals:**
- No new ISA units (e.g. SSE2 scalar fallback for vector functions) — the module's design deliberately links a single ISA unit per build; this change only adds functions to the existing AVX2 unit.
- No matrix-matrix product (the demo is single-token mat-vec only).
- No changes to the existing scalar `fun_math_exp/log/sin/cos` implementations.
- No changes to the demo's remaining hand-rolled numerics (clamped swiglu formula, RoPE interpolation math).

## Decisions

### 1. Function signatures follow the demo's proven contracts
- `fun_math_matrix_vector_f32(const float *w, const float *x, const float *bias, float *out, size_t rows, size_t cols)` — `bias` nullable (`NULL` = no bias). Same row-major `w[r*cols+c]` layout and `out[r] = bias[r] + dot(row_r, x)` contract the demo already uses, so refactoring is mechanical.
- `fun_math_dot_f32(const float *a, const float *b, size_t n)` — replaces `mat_vec_dot32`.
- **Alternative considered**: separate `..._bias` and `..._nobias` functions — rejected, NULL-pointer bias is simpler and the demo always passes a bias for projections.

### 2. Naming: fully descriptive `fun_math_matrix_vector_f32`
AGENTS.md requires descriptive names (e.g. `fun_stream_create_file_read`). `mat_vec` was rejected by the user as too abbreviated; `matrix_vector` matches the convention. `dot` kept as a standard math term.

### 3. Batch transcendentals implemented in the AVX2 unit, scalar tail reuses scalar functions
`exp_f32/log_f32/sin_f32/cos_f32` follow the file's established pattern: unaligned `_mm256_loadu_ps`/`storeu_ps` main loop, scalar tail calling `fun_math_exp/log/sin/cos` for `n % 8`. Using the scalar functions in the tail keeps AVX2 vs scalar consistency tight and avoids duplicating edge-case logic.

### 4. exp_f32 gets saturation blending not present in private `_mm256_exp_ps`
The private helper computes `(k+127)<<23` without clamping; for inputs ≥ 88.72 the exponent field overflows and produces garbage. The public `fun_math_exp_f32` therefore pre-clamps `x` (vectorized min/max against ±88.72 thresholds) before the Pade path, then blends: `+inf in → +inf`, `-inf in → 0`, NaN in → NaN (the Pade path already propagates NaN). This preserves the demo's SWA `-inf` softmax mask.

### 5. log_f32/sin_f32/cos_f32 edge handling via compares + blends
- `log_f32`: mask `x ≤ 0` → `-inf` pattern (or NaN for negative via sign-bit check), `+inf` → `+inf`, else bit-extraction log (same exponent/mantissa algorithm as scalar). Non-SIMD branches for tail elements.
- `sin_f32`/`cos_f32`: same range reduction (mod 2π) and Taylor polynomial as scalar; float→int conversion uses `_mm256_cvttps_epi32` (saturating), safe for the guaranteed domain |x| ≤ 50. `cos` computed as `sin(x + π/2)` like the scalar, for identical accuracy behavior.
- **Alternative considered**: fused sincos helper for RoPE — rejected, keep API surface minimal; the demo batches sin and cos separately.

### 6. Demo refactor sites (demos/gpt-demo/model.c)
- Delete `mat_vec_f32` (was line 203) and `mat_vec_dot32` (was line 226); all call sites switch to the module functions.
- RoPE: precompute `theta_eff[hd/2]` per head into a temp array, then one `fun_math_sin_f32` + `fun_math_cos_f32` call each (vectorized across the 32 dims) instead of per-element scalar sin/cos.
- Attention softmax: replace per-element `scores[t] = fun_math_exp(scores[t] - mx)` with one in-place `fun_math_exp_f32(scores, scores, pos+1)` (the `-inf` SWA mask saturates to 0 via decision 4).
- MoE gate: precompute clamped `-1.702f*g` into a temp array, one `fun_math_exp_f32` call per expert, then `mid[i] = g/(1+e)*(u+1)`.
- The per-position `scores` allocation stays heap-allocated (module does not allocate).

### 7. Tests follow existing patterns
- `test_vector_accuracy.c`: golden-header driven for exp/log/sin/cos `_f32` (extend `tests/math/tools/generate_golden.c`); matrix-vector/dot validated against in-test naive reference loops (no golden file — reference is trivial).
- `test_consistency.c`: SIMD vs scalar per-element comparisons.
- `test_edge_cases.c`: n=0, NaN/±inf/±0, unaligned buffers.
- `test_performance.c`: cycle-per-element entries.
- Build scripts unchanged (already compile `vector_avx2.o`).

## Risks / Trade-offs

- [exp_f32 pre-clamp changes rounding vs scalar exp at exact saturation boundaries] → Clamp thresholds match scalar's own guards (88.7228/-87.3365); consistency tests use tolerance, and boundary inputs are covered by edge tests.
- [sin/cos_f32 accuracy for |x| near 50 relies on float range reduction] → Same algorithm and domain as scalar `fun_math_sin`, which is already spec'd at 1e-3 for |x| ≤ 50; batch accuracy tests use golden values in that domain.
- [Adding functions to only the AVX2 unit means non-AVX2 builds fail to link if they call them] → Consistent with the module's documented ISA-unit design ("no implicit scalar fallback exists"); callers already must link the AVX2 unit to use any vector function.
- [Demo output could shift slightly due to reordering (batched vs scalar accumulation)] → Softmax/RoPE already go through the same approximants; end-to-end verification compares demo output coherence, and correctness is preserved within tolerance by the consistency tests.
