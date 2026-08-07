## 1. AVX2 Vector Functions (rename symbols)

- [x] 1.1 Rename `_silu_f32_avx2` to `fun_math_silu_f32` in `arch/math/avx2/vector.c`
- [x] 1.2 Rename `_swiglu_f32_avx2` to `fun_math_swiglu_f32`
- [x] 1.3 Rename `_rms_norm_f32_avx2` to `fun_math_rms_norm_f32`
- [x] 1.4 Rename `_softmax_f32_avx2` to `fun_math_softmax_f32`
- [x] 1.5 Keep `_mm256_exp_ps`, `_mm256_sigmoid_ps`, `_mm256_silu_ps`, `_mm256_hsum_ps` as static helpers (not public)

## 2. Behavior of Vector Functions

- [x] 2.1 Use unaligned loads (`_mm256_loadu_ps`) and unaligned stores (`_mm256_storeu_ps`) - callers do not guarantee 32-byte alignment
- [x] 2.2 Handle any `n` - vectorized main loop plus scalar tail loop for `n % 8 != 0`
- [x] 2.3 Correct `_mm256_sigmoid_ps`: `sigmoid(x) = 1 / (1 + exp(-x))` with ±20 clamp comparison (no inverted blend)

## 3. Remove Dispatch Layer

- [x] 3.1 Delete `src/math/math_dispatch.c`
- [x] 3.2 Remove `_math_dispatch_init()` call from `src/math/math_init.c`
- [x] 3.3 Remove `_math_dispatch_init` forward declaration from `arch/math/windows-amd64/cpu_features.c`

## 4. Build Scripts

- [x] 4.1 Update `tests/math/build-windows-amd64.bat` - remove `math_dispatch.c`, keep `vector_avx2.o`
- [x] 4.2 Update `tests/math/build-linux-amd64.sh` - same
- [x] 4.3 Update `demos/gpt-demo/build-windows-amd64.bat` - same

## 5. Stack Alignment (GCC/MinGW)

- [x] 5.1 Compile `vector.c` at `-O3 -mavx2 -mfma` - fixes GCC emitting 32-byte-aligned stack spills (`vmovaps`) without a realignment prologue on MinGW (`-mpreferred-stack-boundary` capped at 4 = 16 bytes)

## 6. Verification

- [x] 6.1 Build and run math tests on Windows - all pass, exit code 0
- [x] 6.2 Build and run math tests on Linux (Alpine WSL)
- [x] 6.3 Build and run demo on Windows - model loads, forward pass runs
- [x] 6.4 Run code formatter
- [ ] 6.5 Commit and archive change `remove-math-dispatch`