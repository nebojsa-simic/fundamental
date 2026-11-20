# Tasks

## 1. Public headers and portable kernels

- [x] 1.1 In `include/fundamental/math/math.h`, rename `fun_math_swiglu_f32`, `fun_math_rows_dot_f32`, `fun_math_weighted_sum_f32`, `fun_math_matrix_vector_mxfp4_f32`, `fun_math_matrix_vector_mxfp4_f32_strided`, `fun_math_q8_matrix_vector_f32`, and `fun_math_q8_dequant_row_f32` to the `_cpu` suffix; declare `fun_math_sub_f32`, `fun_math_div_f32`, `fun_math_min_f32`, `fun_math_mean_f32`, and `fun_math_f32_to_fp16`. Verify the header compiles standalone (`gcc -fsyntax-only` on a translation unit including only `math.h`).
- [x] 1.2 In `src/math/math_scalar.c`, implement `fun_math_sub_f32`, `fun_math_div_f32`, `fun_math_min_f32`, `fun_math_mean_f32` (contiguous float32 loops) and `fun_math_f32_to_fp16` (round-to-nearest-even with NaN/inf/subnormal/overflow handling). Verify `fun_math_min_f32(x, 0)` returns 0 and `fun_math_mean_f32(x, 0)` returns 0.

## 2. Rename built-ins across arch units

- [x] 2.1 Rename the built-in definitions in `arch/math/scalar/vector.c` to the `_cpu` suffix (`swiglu_f32_cpu`, `rows_dot_f32_cpu`, `weighted_sum_f32_cpu`, `q8_dequant_row_f32_cpu`, `q8_matrix_vector_f32_cpu`). Verify the file compiles as part of the scalar build.
- [x] 2.2 Apply the same renames in `arch/math/avx2/vector.c`. Verify the AVX2 build compiles.
- [x] 2.3 In `arch/math/avx512/vector.c`, rename `fun_math_swiglu_f32`, `fun_math_rows_dot_f32`, `fun_math_weighted_sum_f32`, `fun_math_q8_matrix_vector_f32`, `fun_math_matrix_vector_mxfp4_f32`, and `fun_math_matrix_vector_mxfp4_f32_strided` to the `_cpu` suffix. Verify the AVX-512 build compiles.
- [x] 2.4 Rename `fun_math_matrix_vector_mxfp4_f32` and `_strided` to the `_cpu` suffix in `arch/math/scalar/vector_mxfp4.c` and `arch/math/avx2/vector_mxfp4.c`. Verify each build that links these objects compiles.

## 3. Backend vtable, wrappers, and dispatch

- [x] 3.1 In `include/fundamental/math/math_kernels.h`, add `SUB`/`DIV`/`MIN`/`MEAN` to `FunMathOp`, append the seven structured entries to `FunMathBackend`, and declare the seven clean-named dispatch wrappers (`fun_math_swiglu_f32`, `fun_math_matrix_vector_mxfp4_f32`, `fun_math_matrix_vector_mxfp4_f32_strided`, `fun_math_q8_matrix_vector_f32`, `fun_math_q8_dequant_row_f32`, `fun_math_rows_dot_f32`, `fun_math_weighted_sum_f32`). Verify the header compiles standalone.
- [x] 3.2 In `src/math/math_kernels.c`, wire the seven new entries into `g_cpu_backend` and implement the seven dispatch wrappers that call the backend entry when set, else the `_cpu` built-in. Verify the default backend produces identical results to the built-ins.
- [x] 3.3 In `src/math/math_kernels.c`, extend `_cpu_unary` to forward contiguous `EXP`/`LOG`/`SIN`/`COS`/`SILU` to the batch kernels, and extend `_cpu_binary`/`_cpu_reduce` to handle `SUB`/`DIV`/`MIN`/`MEAN` for both contiguous and strided tensors. Verify contiguous `unary(EXP)` equals `fun_math_exp_f32`.
- [x] 3.4 In `src/math/math_kernels.c`, implement `F16` (convert-compute-convert via `fun_math_fp16_to_f32`/`fun_math_f32_to_fp16`) and `F64` (double builtins) in the default unary/binary/reduce. Verify `unary(EXP, F16)` matches the convert-compute-convert reference and `unary(SQRT, F64)` matches the double reference within 1e-9.

## 4. Tests

- [x] 4.1 Update `tests/math/test_harness.h` to declare the `_cpu` built-ins, the new kernels (`sub`/`div`/`min`/`mean`), and `fun_math_f32_to_fp16`. Verify the test translation unit compiles.
- [x] 4.2 Extend `tests/math/test_dispatch.c` with suites for: new binary/reduce ops, F16 unary/binary/reduce, F64 unary/binary/reduce, the seven new structured wrappers (wrapper equals `_cpu`, and a stub backend intercepts), the contiguous-unary SIMD fast-path, and the non-F32/F16/F64 dtype no-op. Verify `test.exe` exits 0.
- [x] 4.3 Update any remaining call sites in `tests/math/` that referenced the pre-rename built-in names. Verify the full math suite builds and passes on Windows (scalar, AVX2, AVX-512 builds) with exit code 0.

## Workflow follow-up

- Archive the change with `openspec archive extend-math-dispatch -y` after QA passes and review shows no blockers.
