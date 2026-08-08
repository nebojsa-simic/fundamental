## Why

The gpt-demo uncovered blind spots in the math module: matrix-vector multiplication and dot product — the core operations of neural network inference — do not exist in the public API, forcing the demo to hand-roll raw AVX2 intrinsics in `model.c`. Additionally, `exp`, `log`, `sin`, and `cos` exist only as scalar functions, so the demo calls them per-element in the hottest loops (RoPE, attention softmax, MoE gating), leaving ~2x performance on the table.

## What Changes

- Add `fun_math_matrix_vector_f32(w, x, bias, out, rows, cols)` — biased matrix-vector product with optional bias (`NULL` allowed), AVX2 FMA + scalar tail.
- Add `fun_math_dot_f32(a, b, n)` — vector dot product, AVX2 FMA + scalar tail.
- Add vectorized transcendental batch functions: `fun_math_exp_f32`, `fun_math_log_f32`, `fun_math_sin_f32`, `fun_math_cos_f32` — AVX2 main loop with scalar tail for `n % 8`.
- `fun_math_exp_f32` saturates identically to scalar `fun_math_exp` (`-inf → 0`, overflow → `+inf`, underflow → `0`) so masked softmax inputs (e.g. the demo's SWA `-inf` mask) behave correctly.
- `fun_math_log_f32` matches scalar edge semantics (0 → `-inf`, negative → NaN, `+inf` → `+inf`).
- Refactor `demos/gpt-demo/model.c` to consume the new API and delete its hand-rolled `mat_vec_f32` and `mat_vec_dot32`: projections, attention score dot products, RoPE sin/cos batches, attention softmax exp batch, and MoE gate exp batch.
- Extend `tests/math` with accuracy, consistency, edge-case, and performance coverage for the new functions.

## Capabilities

### New Capabilities

<!-- None: the project uses a flat spec layout; all math behavior lives in the existing `math` capability. -->

### Modified Capabilities

- `math`: existing `math` capability gains the new matrix-vector, dot product, and batched transcendental vector functions in its public API surface.

## Impact

- `include/fundamental/math/math.h` — 6 new public declarations
- `arch/math/avx2/vector.c` — new implementations (exp/log/sin/cos `_ps` helpers promoted/internal, saturation handling)
- `demos/gpt-demo/model.c` — drop `mat_vec_f32`/`mat_vec_dot32`, call module functions; RoPE/softmax/MoE gate batch calls
- `tests/math/*` — new test coverage (vector accuracy, consistency, edge cases, performance)
- No build script changes: test and demo builds already compile `arch/math/avx2/vector.c` with `-mavx2 -mfma`
- Follows the module's existing ISA-unit design: vector functions provided by `arch/math/avx2/vector.c`, link-time selection, no runtime dispatch
