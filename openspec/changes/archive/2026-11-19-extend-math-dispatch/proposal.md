# Proposal

## Why

The backend dispatch layer added in `generalized-math-kernels` only routes a
subset of the math module through the `FunMathBackend` vtable: the elementwise
binary/reduction ops and five structured kernels. The remaining structured
kernels (swiglu, MXFP4/Q8 matrix-vector, Q8 dequantization, row-wise dot, and
weighted sum) still call the built-in directly, the reserved `F16`/`F64` dtypes are silent
no-ops, the generic unary path is scalar-only, and the op set is missing
sub/div/min/mean. This closes those seams so the dispatch layer covers the
whole kernel surface a future accelerator backend would need to override.

## What Changes

- Route the remaining structured kernels through the backend as vtable entries
  with dispatch wrappers: `swiglu_f32`, `matrix_vector_mxfp4_f32` and its
  strided variant, `q8_matrix_vector_f32`, `q8_dequant_row_f32`,
  `rows_dot_f32`, and `weighted_sum_f32`. Built-ins are renamed to the `_cpu`
  suffix convention (`fun_math_<name>_cpu`), and the clean names become
  dispatch wrappers that fall back to the built-in. **BREAKING** for callers of
  the built-in symbol names (clean names keep working and now route through the
  backend); `gpt-demo` and the tests already call the clean names, so they are
  unaffected.
- Add `SUB` and `DIV` binary ops and `MIN` and `MEAN` reduction ops to
  `FunMathOp`, plus portable contiguous kernels `fun_math_sub_f32`,
  `fun_math_div_f32`, `fun_math_min_f32`, and `fun_math_mean_f32`.
- Give the default CPU backend's generic unary path a SIMD fast-path for the
  contiguous case, forwarding `EXP`/`LOG`/`SIN`/`COS`/`SILU` to the existing
  batch kernels (`fun_math_exp_f32` etc.); ops without a batch kernel stay
  scalar.
- Implement real `F16` and `F64` behavior in the generic dispatch. `F16` tensors
  hold `uint16_t` and are converted via the existing `fun_math_fp16_to_f32` and
  a new `fun_math_f32_to_fp16`. `F64` tensors hold `double` and are computed
  with the compiler's `__builtin_*` double-precision math builtins.

## Capabilities

### New Capabilities

(none)

### Modified Capabilities

- `math`: extend the backend registry/dispatch, operation enumeration, data type
  enumeration, and add the new portable binary/reduction kernels.

## Impact

- `include/fundamental/math/math_kernels.h`: append seven structured entries to
  `FunMathBackend`; declare the seven new dispatch wrappers; extend
  `FunMathOp` with `SUB`/`DIV`/`MIN`/`MEAN`.
- `include/fundamental/math/math.h`: rename seven built-in declarations to the
  `_cpu` suffix; declare `fun_math_sub_f32`/`div_f32`/`min_f32`/`mean_f32` and
  `fun_math_f32_to_fp16`.
- `src/math/math_kernels.c`: wire the default backend, add the seven wrappers,
  extend the default unary/binary/reduce to handle `F16`/`F64` and the new ops,
  add the contiguous unary SIMD fast-path.
- `src/math/math_scalar.c`: add the four new kernels and `fun_math_f32_to_fp16`.
- `arch/math/{scalar,avx2,avx512}/vector.c`: rename the seven built-in
  definitions to `_cpu` (note the AVX-512 unit also defines the MXFP4 kernels,
  which the other two units place in `arch/math/{scalar,avx2}/vector_mxfp4.c`).
- `tests/math/`: extend `test_dispatch.c` and the harness; no build-script
  source-list changes (all sources already compiled).
- `demos/gpt-demo`: no changes required (clean names become wrappers).
