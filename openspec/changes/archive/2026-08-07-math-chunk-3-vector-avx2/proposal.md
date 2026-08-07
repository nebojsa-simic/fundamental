## Why

Four vector functions remain as stubs in math_scalar.c: silu_f32, rms_norm_f32, swiglu_f32, softmax_f32. This causes ~37K golden data tests + ~143K consistency tests to fail. The scalar implementations (Chunks 1+2) provide the building blocks, but SIMD is required for the LLM inference hot path where these functions process millions of elements. This chunk implements AVX2 for all four functions, with runtime dispatch and scalar fallback for non-AVX2 CPUs.

## What Changes

- New `arch/math/avx2/vector.c` — AVX2 implementations with internal exp/sigmoid/silu helpers, no tail handling (API requires aligned buffers + n%8==0)
- New `src/math/math_dispatch.c` — function pointer table, public vector entry points, scalar fallbacks, dispatch setup
- Modify `src/math/math_init.c` — call dispatch setup after CPU detection
- Remove 4 vector stubs from `src/math/math_scalar.c`
- Modify build scripts: compile `vector.c` with `-mavx2 -mfma`, link separately
- API contract: buffers must be 32-byte aligned, n must be multiple of 8 on AVX2 path

## Capabilities

### Modified Capabilities

- `math`: All 8 scalar and 4 vector functions have real implementations with AVX2 dispatch and scalar fallback.

## Impact

- New files: `arch/math/avx2/vector.c`, `src/math/math_dispatch.c`
- Modified: `src/math/math_init.c`, `src/math/math_scalar.c`, `tests/math/build-windows-amd64.bat`, `tests/math/build-linux-amd64.sh`
- Tests: ~37K vector accuracy + ~143K consistency tests → all passing
- Exit code: 0
- No header changes
