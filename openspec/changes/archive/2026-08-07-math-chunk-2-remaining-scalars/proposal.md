## Why

The math module currently has sqrt, exp, and log implemented (Chunk 1). Five scalar functions remain as stubs returning 0.0: sin, cos, tanh, sigmoid, silu. This causes ~8000 golden data tests + 12 edge case tests to fail. The tests, golden data headers, public API header, and build scripts are all already in place — only the implementations in `math_scalar.c` are missing. Three of the five functions (tanh, sigmoid, silu) compose naturally from the already-working `fun_math_exp`, making this a low-risk implementation.

## What Changes

- Replace 5 stub functions in `src/math/math_scalar.c` with real scalar implementations
- Implement `fun_math_sin` — range reduction to [−π/2, π/2] + 7th-degree Taylor polynomial
- Implement `fun_math_cos` — `cos(x) = sin(x + π/2)`, reusing the sin implementation
- Implement `fun_math_tanh` — `tanh(x) = (e^(2x) − 1) / (e^(2x) + 1)` using `fun_math_exp`, with saturation clamping for |x| ≥ 10
- Implement `fun_math_sigmoid` — `σ(x) = 1 / (1 + e^(-x))` using `fun_math_exp`, with saturation clamping
- Implement `fun_math_silu` — `silu(x) = x · σ(x)` using `fun_math_sigmoid`, with correct ±inf handling
- No new files, no header changes, no build script changes, no test changes

## Capabilities

### Modified Capabilities

- `math`: Complete scalar function surface — all 8 scalar functions (sqrt, exp, log, sin, cos, tanh, sigmoid, silu) now have real implementations. Vector functions remain stubbed for Chunk 3.

## Impact

- Modifies: `src/math/math_scalar.c` — replace 5 stubs (lines 123-151) with real implementations
- Tests: ~8000 accuracy tests + 12 edge case tests go from failing to passing
- Exit code: test harness goes from code 1 (failures in stubbed functions) to code 0 (all scalar tests pass)
- No breaking changes to existing APIs
- Depends on: `fun_math_exp` (already implemented in same file)
- No new source files, no build script changes
