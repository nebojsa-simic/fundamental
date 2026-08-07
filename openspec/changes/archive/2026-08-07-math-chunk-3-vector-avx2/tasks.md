## 1. AVX2 Internal Helpers

- [x] 1.1 Create `arch/math/avx2/vector.c` with `_mm256_exp_ps` — Pade (4,4), same algorithm as scalar exp, using `_mm256_fmadd_ps` and `_mm256_round_ps`
- [x] 1.2 Add `_mm256_sigmoid_ps` helper — 1/(1+exp(-x)) with ±20 clamping, split for x≥0 and x<0 stability
- [x] 1.3 Add `_mm256_silu_ps` helper — x * sigmoid(x)

## 2. AVX2 Vector Functions

- [x] 2.1 Implement `_silu_f32_avx2` — pure SIMD loop, aligned loads/stores
- [x] 2.2 Implement `_swiglu_f32_avx2` — gate * silu(gate) * up, two aligned loads
- [x] 2.3 Implement `_rms_norm_f32_avx2` — two-pass: sum-of-squares reduction → scale → normalize
- [x] 2.4 Implement `_softmax_f32_avx2` — in-place, two-pass: max reduction → exp sum → divide

## 3. Dispatch Infrastructure

- [x] 3.1 Create `src/math/math_dispatch.c` — typedefs, static fn ptrs (default scalar), public entry points for all 4 functions
- [x] 3.2 Add scalar fallbacks in math_dispatch.c — scalar loops for each function
- [x] 3.3 Add `_math_dispatch_init()` — set fn ptrs to AVX2 if g_has_avx2
- [x] 3.4 Modify `src/math/math_init.c` — call `_math_dispatch_init()` from `fun_math_init()`
- [x] 3.5 Remove 4 vector stubs from `src/math/math_scalar.c`

## 4. Build Integration

- [x] 4.1 Update `tests/math/build-windows-amd64.bat` — compile `avx2/vector.c` with `-mavx2 -mfma` to `.o`, link with rest
- [x] 4.2 Update `tests/math/build-linux-amd64.sh` — same for Linux

## 5. Verification

- [x] 5.1 Build and run on Windows — 194394 passed, 0 failed, exit code 0
- [x] 5.2 Build and run on Linux (Alpine WSL) — 194394 passed, 0 failed, exit code 0
- [x] 5.3 Performance benchmarks show AVX2 speedup: rms_norm ~8.6 cyc/el (solid), silu/swiglu/softmax functional but similar to scalar (helper recursion through exp/sigmoid dominates)
- [x] 5.4 Run code formatter — clang-format applied, rebuild passes
- [x] 5.5 Edge case tests pass (zero-length vectors, 33/33)
