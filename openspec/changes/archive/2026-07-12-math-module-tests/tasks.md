## 1. Test Infrastructure

- [x] 1.1 Create `tests/math/` directory structure with subdirectories `test_data/` and `tools/`
- [x] 1.2 Create `tests/math/test_main.c` — test harness runner that calls all suites, accumulates pass/fail counts, prints summary, returns exit code 0 or 1
- [x] 1.3 Create `tests/math/test_harness.h` — shared types (`TestCount` struct), float utilities (NaN/Inf/neg zero helpers), LCG pseudo-random generator (`_math_test_lcg_seed`, `_math_test_lcg_next`), error checking helpers

## 2. Golden Value Generation Tool

- [x] 2.1 Create `tests/math/tools/generate_golden.c` — C program (using stdlib math.h) that generates C header files with precomputed test vectors
- [x] 2.2 Use `float` operations (e.g., `sqrtf`, `expf`) in the generator so expected values match single-precision arithmetic
- [x] 2.3 Generate golden headers for scalar functions: `sqrt_golden.h`, `exp_golden.h`, `log_golden.h`, `sin_golden.h`, `cos_golden.h`, `tanh_golden.h`, `sigmoid_golden.h`, `silu_golden.h`
- [x] 2.4 Generate golden headers for vector functions: `silu_f32_golden.h`, `rms_norm_f32_golden.h`, `swiglu_f32_golden.h`, `softmax_f32_golden.h`
- [x] 2.5 Include one deliberately incorrect golden case in `harness_self_test_golden.h` for harness self-validation

## 3. Scalar Accuracy Tests

- [x] 3.1 Create `tests/math/test_scalar_accuracy.c` — loads golden headers, iterates test cases, compares function output to expected value using relative/absolute error with per-case tolerance
- [x] 3.2 Define forward declarations for all scalar math functions (`fun_math_sqrt`, `fun_math_exp`, `fun_math_log`, `fun_math_sin`, `fun_math_cos`, `fun_math_tanh`, `fun_math_sigmoid`, `fun_math_silu`) matching the API contract

## 4. Vector Accuracy Tests

- [x] 4.1 Create `tests/math/test_vector_accuracy.c` — loads golden headers, iterates test cases for batch functions, compares element-by-element
- [x] 4.2 Define forward declarations for all vector math functions (`fun_math_silu_f32`, `fun_math_rms_norm_f32`, `fun_math_swiglu_f32`, `fun_math_softmax_f32`) matching the API contract
- [x] 4.3 Include softmax sum-to-one check (output elements must sum to 1.0 within 1e-5)

## 5. Edge Case Tests

- [x] 5.1 Create `tests/math/test_edge_cases.c` — tests special float values: ±0.0, ±infinity, NaN, extreme magnitudes
- [x] 5.2 Test sqrt edge cases: ±0.0, +inf, negative input (should return NaN), NaN input
- [x] 5.3 Test exp edge cases: +inf, -inf, NaN, overflow at ~88.7, underflow at ~-88.7
- [x] 5.4 Test log edge cases: +0.0 (should return -inf), -0.0 (same), negative input (NaN), +inf, NaN
- [x] 5.5 Test zero-length vector calls: silu_f32, rms_norm_f32, swiglu_f32, softmax_f32 with n=0

## 6. Consistency Cross-Validation Tests

- [x] 6.1 Create `tests/math/test_consistency.c` — compares SIMD vector output against scalar reference on identical inputs
- [x] 6.2 Test silu_f32 consistency: generate 100 random input arrays of length 1024, call SIMD and scalar, compare element-by-element
- [x] 6.3 Test rms_norm_f32 consistency: generate 100 random input/weight arrays of length 4096, call SIMD and scalar, compare element-by-element

## 7. Performance Benchmarks

- [x] 7.1 Create `tests/math/test_performance.c` — RDTSC-based cycle counting for each scalar and vector function
- [x] 7.2 Implement `_math_test_rdtsc()` inline assembly helper for x86 (`rdtsc`)
- [x] 7.3 Benchmark scalar functions: 100K iterations on 1024-element arrays, report cycles/element
- [x] 7.4 Benchmark vector functions: 10K iterations on 65536-element arrays, report cycles/element
- [x] 7.5 Include a dummy no-op benchmark to measure loop overhead

## 8. Harness Self-Test

- [x] 8.1 Include test that runs against `harness_self_test_golden.h` and verifies it detects the deliberate failure
- [x] 8.2 Verify the harness exit code is non-zero when self-test golden case fails (confirmed: exit code 1)

## 9. Build Scripts

- [x] 9.1 Create `tests/math/build-windows-amd64.bat` — compiles test harness with gcc, includes all test `.c` files plus math_stubs.c, strips binary
- [x] 9.2 Create `tests/math/build-linux-amd64.sh` — same structure for Linux
- [x] 9.3 Verify build scripts compile cleanly (confirmed: Windows build succeeds with zero errors)

## 10. Integration Validation

- [x] 10.1 Run `build-windows-amd64.bat` — verify clean compilation with zero errors
- [x] 10.2 Run `test.exe` — verify all tests execute (confirmed: all 6 suites execute, harness self-test detects failures, exit code is non-zero with stubs)
- [x] 10.3 Build and run the golden value generator — verify headers are valid C and parse correctly
- [x] 10.4 Run `run-tests-windows-amd64.bat` — math tests integrate with the global test runner (directory is discovered; build succeeds; tests exit non-zero with stubs as expected)
