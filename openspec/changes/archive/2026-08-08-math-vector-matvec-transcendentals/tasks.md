## 1. Public API

- [x] 1.1 Add `fun_math_matrix_vector_f32`, `fun_math_dot_f32`, `fun_math_exp_f32`, `fun_math_log_f32`, `fun_math_sin_f32`, `fun_math_cos_f32` declarations to `include/fundamental/math/math.h`

## 2. AVX2 Implementations

- [x] 2.1 Implement `fun_math_matrix_vector_f32` in `arch/math/avx2/vector.c` (8-wide FMA loop + scalar tail, nullable bias)
- [x] 2.2 Implement `fun_math_dot_f32` in `arch/math/avx2/vector.c` (FMA loop + scalar tail)
- [x] 2.3 Implement `fun_math_exp_f32` with saturation blending (clamp, ±inf/NaN handling) using the existing `_mm256_exp_ps` helper
- [x] 2.4 Implement `fun_math_log_f32` (bit-extraction log + edge masks for 0/negative/+inf)
- [x] 2.5 Implement `fun_math_sin_f32` and `fun_math_cos_f32` (range reduction + polynomial, saturated float->int conversion)

## 3. gpt-demo Refactor

- [x] 3.1 Replace `mat_vec_f32`/`mat_vec_dot32` call sites in `model.c` with `fun_math_matrix_vector_f32`/`fun_math_dot_f32` and delete the hand-rolled helpers
- [x] 3.2 Batch RoPE sin/cos via `fun_math_sin_f32`/`fun_math_cos_f32` over `theta_eff[hd/2]` arrays
- [x] 3.3 Batch attention softmax exp via in-place `fun_math_exp_f32` over the scores array
- [x] 3.4 Batch MoE gate exp via `fun_math_exp_f32` over the clamped `-1.702*g` array per expert

## 4. Tests

- [x] 4.1 Extend `tests/math/tools/generate_golden.c` with exp/log/sin/cos `_f32` golden case generation
- [x] 4.2 Add exp/log/sin/cos `_f32` accuracy blocks to `test_vector_accuracy.c`
- [x] 4.3 Add matrix-vector/dot accuracy blocks to `test_vector_accuracy.c` (naive in-test reference)
- [x] 4.4 Add SIMD-vs-scalar consistency cases to `test_consistency.c` for all new functions
- [x] 4.5 Add edge cases (n=0, NaN/±inf/±0, unaligned) to `test_edge_cases.c`
- [x] 4.6 Add performance benchmark entries to `test_performance.c`

## 5. Build & Verification

- [x] 5.1 Build and run `tests\math\build-windows-amd64.bat` + `test.exe` — all suites pass
- [x] 5.2 Build and run `demos\gpt-demo\build-windows-amd64.bat` + `demo.exe "What is 2+2?"` — coherent output, no regressions
- [x] 5.3 Run `code-format.bat` on changed files
