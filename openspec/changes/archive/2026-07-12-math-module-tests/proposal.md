## Why

The LLM execution engine depends on floating-point math primitives (sqrt, exp, log, sin, cos, tanh, sigmoid, silu, RMS norm, softmax, SwiGLU). No math module exists in the library today — there are zero transcendental functions, zero vector operations, and no performance baseline. Starting with tests first establishes the correctness contract and performance targets before any implementation code is written.

## What Changes

- New `tests/math/` directory with comprehensive test suites covering scalar functions, vector functions, edge cases, and performance benchmarks
- Golden value headers (`tests/math/test_data/`) generated offline by a Python tool (`tests/math/tools/generate_golden.py`)
- Build scripts for `windows-amd64` (latest: AVX-512) and `linux-amd64` (latest: AVX-512)
- Test structure defines the API contract — function names, signatures, and error bounds are locked in by the tests
- Older feature sets (avx2, sse2, arm64-neon below SVE, etc.) deferred to later phases

## Capabilities

### New Capabilities

- `math`: Scalar and vectorized floating-point math primitives for LLM inference. Includes transcendental functions (sqrt, exp, log, sin, cos, tanh, sigmoid, silu), vector operations (silu_f32, rms_norm_f32, swiglu_f32, softmax_f32), and a performance benchmarking harness. Tests validate accuracy against IEEE 754 reference golden values, edge case handling, and cycle-count performance.

### Modified Capabilities

<!-- None — purely additive module -->

## Impact

- New test directory: `tests/math/` with golden data headers, build scripts, and test C sources
- New tool: `tests/math/tools/generate_golden.py` (offline, not part of the build)
- Depends on: `memory` module (`fun_memory_allocate`, `fun_memory_free`), `console` module (test output)
- No source or header files created — this phase is tests-only
- Defines the public API surface that Phase 2 (implementation) must satisfy
- No breaking changes to existing APIs
