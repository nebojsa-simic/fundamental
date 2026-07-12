## 1. Public API Header

- [x] 1.1 Create `include/fundamental/math/math.h` with all 8 scalar and 4 vector function declarations, include guards, and CPU feature query prototypes

## 2. CPU Feature Detection

- [x] 2.1 Create `arch/math/windows-amd64/cpu_features.c` — CPUID-based detection for SSE2, AVX, AVX2, AVX-512F using inline asm (avoids `_xgetbv` intrinsic target mismatch)
- [x] 2.2 Create `arch/math/linux-amd64/cpu_features.c` — same detection using inline asm `cpuid` instruction
- [x] 2.3 Create `src/math/math_init.c` — weak `fun_math_init()`, feature state storage, `_math_set_features()` internal hook called by arch detection code

## 3. Scalar Implementation

- [x] 3.1 Create `src/math/math_scalar.c` with sqrt, exp, log implementations, plus stubs for sin/cos/tanh/sigmoid/silu returning 0.0
- [x] 3.2 Implement `fun_math_sqrt` — fast inverse sqrt seed + two Newton-Raphson iterations (1 iteration was insufficient for 1e-4 tolerance), sign-preserving zero handling and NaN propagation
- [x] 3.3 Implement `fun_math_exp` — range reduction via `x / ln(2)`, split integer/fractional, Pade (4,4) approximant for fractional part, overflow/underflow saturation
- [x] 3.4 Implement `fun_math_log` — exponent/mantissa extraction, atanh series (5 terms) for mantissa in [1,2), edge case handling for zero/negative/inf/NaN

## 4. Build Integration

- [x] 4.1 Update `tests/math/build-windows-amd64.bat` — replace `math_stubs.c` with `src/math/math_scalar.c`, add `arch/math/windows-amd64/cpu_features.c` and `src/math/math_init.c`
- [x] 4.2 Update `tests/math/build-linux-amd64.sh` — same replacement for Linux paths
- [x] 4.3 Delete `tests/math/math_stubs.c` — no longer needed

## 5. Verification

- [x] 5.1 Build and run on Windows — sqrt/exp/log accuracy and edge cases pass, harness self-test passes (2/2)
- [x] 5.2 Verify deferred functions (sin/cos/tanh/sigmoid/silu) still fail with 0.0 return as expected
- [x] 5.3 sqrt/exp/log edge cases all pass (17/17). Scalar accuracy up from 948 to 6101 passes. Exit code 1 due to deferred stubs (expected)
- [x] 5.4 Performance benchmarks: sqrt 11 cyc/el, exp 29 cyc/el, log 16 cyc/el — all < 100 cyc/el
