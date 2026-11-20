# Design

## Context

The `math` module's backend dispatch layer (`include/fundamental/math/math_kernels.h`, `src/math/math_kernels.c`) currently routes the elementwise binary/reduction ops and five structured kernels (matrix-vector, dot, rms_norm, softmax, rotary) through a `FunMathBackend` vtable. The remaining structured kernels, the reserved `F16`/`F64` dtypes, the op set, and the unary fast-path were all deferred in the prior change. See proposal.md for the motivation.

The established convention is that the clean name (`fun_math_softmax_f32`) is the dispatch wrapper and the `_cpu` suffix (`fun_math_softmax_f32_cpu`) is the built-in kernel. `gpt-demo` and the tests already call the clean names for the structured kernels.

## Goals / Non-Goals

**Goals:**
- Route every remaining f32 structured kernel through the backend with dispatch wrappers.
- Extend the generic dispatch to handle `F16`/`F64` and the `SUB`/`DIV`/`MIN`/`MEAN` ops.
- Give the contiguous unary dispatch a SIMD fast-path using the existing batch kernels.

**Non-Goals:**
- No GPU/accelerator backend code.
- No f16/f64 support for the *structured* kernels (they remain f32).
- No SIMD for the generic binary/reduce path or for unary ops without a batch kernel.
- No new public f64 scalar transcendental API (builtins used internally).

## Decisions

### D1: Clean name is the wrapper, `_cpu` is the built-in (rename, not add)
The seven unrouted built-ins (`swiglu_f32`, `matrix_vector_mxfp4_f32`, `matrix_vector_mxfp4_f32_strided`, `q8_matrix_vector_f32`, `q8_dequant_row_f32`, `rows_dot_f32`, `weighted_sum_f32`) are renamed to the `_cpu` suffix, and new clean-named dispatch wrappers are added in `math_kernels.c`. This matches the existing convention and gives `gpt-demo` backend routing with zero call-site changes.
- *Alternative considered*: additive `_dispatch`-suffixed wrappers. Rejected — inconsistent with the clean-name-is-wrapper convention already documented in the math spec.

### D2: New vtable entries are appended at the end
`FunMathBackend` is a public C struct; entries must be appended (not reordered) to preserve source compatibility for backend implementers. Seven entries are appended after `rotary_f32`: `swiglu_f32`, `matrix_vector_mxfp4_f32`, `matrix_vector_mxfp4_f32_strided`, `q8_matrix_vector_f32`, `q8_dequant_row_f32`, `rows_dot_f32`, `weighted_sum_f32`.

### D3: Op codes grouped; SUM/MAX renumber
`SUB` and `DIV` are inserted into the binary group and `MIN`/`MEAN` into the reduction group, keeping the enum grouped (unary | binary | reduce). This renumbers `MUL`/`SUM`/`MAX`; op codes are not persisted and are only consumed in-process via the enum names, so no compatibility concern.
- *Alternative considered*: append all four after `MAX` to keep existing values stable. Rejected — produces a confusing order (reduction codes interleaved with binary codes).

### D4: F16 is convert-compute-convert
F16 tensors hold `uint16_t`. The default backend's unary/binary/reduce decode each element with `fun_math_fp16_to_f32`, compute in float32, and encode with a new `fun_math_f32_to_fp16` (round-to-nearest-even, NaN/inf/subnormal/overflow handled). `fun_math_f32_to_fp16` lives in `src/math/math_scalar.c` beside `fun_math_fp16_to_f32` and is declared in `math.h`. The generic `FunMathTensor` needs no change (`data` is `void *`, stride is element-based).

### D5: F64 is true double via compiler builtins
F64 tensors hold `double`. Unary transcendentals use `__builtin_sqrt`, `__builtin_exp`, `__builtin_log`, `__builtin_sin`, `__builtin_cos`, `__builtin_tanh`; sigmoid/silu are composed from `__builtin_exp`. Binary/reduce use plain double arithmetic. This matches the codebase's builtins-only style; the double builtins lower to `libm`, which every math build already links (`-lm`).
- *Alternative considered*: narrow `double`→`float`, use the float kernels, widen back. Rejected — silently lossy and not "real" f64.

### D6: The default backend switches on dtype internally
The vtable entries keep their existing `(op, dtype, tensors)` signatures. The default backend's `unary`/`binary`/`reduce` implementations branch on `dtype` (F32/F16/F64) and, within F32, on contiguity and op. `fun_math_unary`/`binary`/`reduce` wrappers continue to just forward to the current backend's entry (or the default on a NULL entry).

### D7: Contiguous unary SIMD fast-path
For `FUN_MATH_DTYPE_F32` with contiguous input and output (stride 1), the default unary forwards `EXP`/`LOG`/`SIN`/`COS`/`SILU` to the existing batch kernels (`fun_math_exp_f32`, `fun_math_log_f32`, `fun_math_sin_f32`, `fun_math_cos_f32`, `fun_math_silu_f32`). The batch kernels already match the scalar per-op semantics (covered by the existing consistency tests), so this is behavior-preserving. `SQRT`/`TANH`/`SIGMOID` and strided tensors keep the scalar loop.

## Risks / Trade-offs

- [Large mechanical rename across five arch files] → The build gate catches any missed reference; the rename is `_cpu`-suffix-only and mechanical.
- [F16 rounding correctness] → `fun_math_f32_to_fp16` must implement round-to-nearest-even; verified against known bit patterns in tests.
- [`__builtin_*` double math portability] → GCC/Clang (MinGW and Linux) provide these builtins; the codebase already relies on `__builtin_sqrtf`/`__builtin_expf`.
- [F64 sigmoid/silu differ from the float approximation] → Expected: F64 is a distinct double-precision path, not a widening of the float approximation.
- [Backend vtable ABI stability] → Append-only; documented in the header.

## Migration Plan

Additive plus a mechanical `_cpu` rename. Existing callers use the clean names and are unaffected (they now route through the backend). Rollback is a revert of the change; no data or on-disk format is involved.

## Open Questions

None.
