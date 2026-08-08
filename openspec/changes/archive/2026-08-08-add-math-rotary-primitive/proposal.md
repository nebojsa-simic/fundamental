# Proposal: Add Math Rotary Primitive

## Why

The gpt-demo's RoPE implementation (`rope_single` in `demos/gpt-demo/model.c`)
applies rotary positional embeddings with a scalar loop that:

1. Recomputes the identical theta series and cos/sin tables for every one of
   the 72 attention heads (64 query + 8 KV heads) even though the result is
   identical across heads.
2. Applies the 2x2 rotation (`q0*ca - q1*sa`, `q0*sa + q1*ca`) one element at
   a time with scalar FMAs, despite being an embarrassingly parallel kernel.
3. Recomputes `theta_scale = exp(log(theta) * -2/hd)` via two scalar calls on
   every invocation.

The library already ships AVX2 vectorized math primitives
(`fun_math_rms_norm_f32`, `fun_math_swiglu_f32`, `fun_math_sin_f32`, ...) in
`arch/math/avx2/vector.c`, but has no primitive for the 2x2 rotation step,
which is the universally reusable part of RoPE.

## What Changes

- New public primitive `fun_math_rotary_f32()` in `include/fundamental/math/math.h`
  implemented in `arch/math/avx2/vector.c` (AVX2 kernel with scalar tail,
  following the existing `_f32` function pattern - no scalar fallback).
- Golden value test data generated from a scalar reference implementation
  (extended `tests/math/tools/generate_golden.c`), tested in
  `tests/math/test_vector_accuracy.c` and `tests/math/test_edge_cases.c`.
- Demo refactor in `demos/gpt-demo`: precompute the rope power/ramp/mscale
  table once at `model_load`, compute cos/sin once per call, apply the
  rotation with the new primitive. Greedy-argmax output is preserved.

## Capabilities

### New Capabilities
- `math.rotary`: AVX2 vectorized 2x2 rotary rotation kernel with scalar tail.

### Modified Capabilities
- `gpt-demo.rope`: `rope_single` restructured to use the precomputed table and
  `fun_math_rotary_f32`; no change to observable generation output.

## Impact

- `include/fundamental/math/math.h` - one new declaration (additive).
- `arch/math/avx2/vector.c` - one new kernel (additive).
- `tests/math/tools/generate_golden.c` - new `gen_vector_rotary()` section.
- `tests/math/test_data/rotary_f32_golden.h` - new generated golden file.
- `tests/math/test_vector_accuracy.c`, `tests/math/test_edge_cases.c` - new
  test functions.
- `demos/gpt-demo/model.h` - `float *rope_pre` field on `Model`.
- `demos/gpt-demo/model.c` - precompute + refactored `rope_single`.
- No breaking changes to existing APIs.
