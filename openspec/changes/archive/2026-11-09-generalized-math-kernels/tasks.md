# Tasks

## 1. New portable elementwise and reduction kernels

- [x] 1.1 Declare `fun_math_add_f32`, `fun_math_mul_f32`, `fun_math_sum_f32`, and `fun_math_max_f32` in `include/fundamental/math/math.h`. Verify the header still compiles by including it from a throwaway TU (`gcc -c --std=c17 -I include -x c -`).
- [x] 1.2 Implement the four kernels as portable scalar loops in `src/math/math_scalar.c`. Verify each matches a naive in-test reference (covered by the dispatch test in group 4).

## 2. Backend and dispatch header

- [x] 2.1 Create `include/fundamental/math/math_kernels.h` defining `FunMathDtype` (F32 implemented, F16/F64 reserved), `FunMathOp` (unary/binary/sum/max), `FunMathTensor` (data, n, element stride), and `FunMathBackend` (unary/binary/reduce + matrix_vector/dot/rms_norm/softmax/rotary entries). Verify the header compiles standalone.
- [x] 2.2 Declare `fun_math_backend_set`, `fun_math_backend_get`, `fun_math_unary`, `fun_math_binary`, `fun_math_reduce`, and the structured dispatch wrappers (`fun_math_matrix_vector_f32_dispatch`, `fun_math_dot_f32_dispatch`, `fun_math_rms_norm_f32_dispatch`, `fun_math_softmax_f32_dispatch`, `fun_math_rotary_f32_dispatch`) in `math_kernels.h`. Verify the header compiles standalone.

## 3. Backend registry and dispatch implementation

- [x] 3.1 Implement `src/math/math_kernels.c` with the registry (`fun_math_backend_set`/`fun_math_backend_get`) and a static default CPU backend. Verify `fun_math_backend_get()` returns a non-NULL default backend.
- [x] 3.2 Implement the default CPU backend's `unary`/`binary`/`reduce` entries as strided elementwise loops over `FunMathTensor`; unary uses the scalar `fun_math_*` functions, and binary/reduce delegate to the new `fun_math_add_f32`/`mul_f32`/`sum_f32`/`max_f32` for the contiguous case. Verify a contiguous `unary(EXP)` result equals per-element `fun_math_exp` (dispatch test).
- [x] 3.3 Implement the structured dispatch wrappers that call the backend entry when set, else the built-in `fun_math_*` kernel. Verify each wrapper equals the built-in kernel under the default backend (dispatch test).

## 4. Tests

- [x] 4.1 Create `tests/math/test_dispatch.c` with suites for: default-backend dispatch equals direct kernel calls, a swapped-in test-stub backend intercepts a structured op, strided-tensor correctness, and the new add/mul/sum/max kernels. Verify it compiles cleanly with the test harness.
- [x] 4.2 Register the new suite in `tests/math/test_main.c`. Verify the suite name appears in test output when the test binary runs.
- [x] 4.3 Wire `test_dispatch.c` and `src/math/math_kernels.c` into `tests/math/build-windows-amd64.bat`, `build-windows-amd64-avx2.bat`, `build-windows-amd64-sse.bat`, and the three matching Linux scripts. Verify each build script produces `test.exe`/`test` with exit code 0.

## 5. Verification

- [x] 5.1 Build and run the math test binary for the scalar, AVX2, and AVX-512 variants; verify all suites pass (exit code 0).
- [x] 5.2 Run `openspec validate generalized-math-kernels` and verify it reports the change as valid with no blockers.

## Workflow follow-up

- Archive the change with `openspec archive generalized-math-kernels` after QA passes and review shows no blockers.
