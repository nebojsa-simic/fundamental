## Why

The math module tests exist and validate 50K+ golden value cases, but every test fails because no implementation exists — the stubs return 0.0 for all functions. This chunk delivers the first working scalar functions (sqrt, exp, log) along with CPU feature detection infrastructure, unlocking ~6,000 test cases and establishing the module architecture that Chunks 2 and 3 will extend.

## What Changes

- New `include/fundamental/math/math.h` — public API header for all math functions
- New `src/math/math_scalar.c` — scalar reference implementations for sqrt, exp, log using polynomial approximations and iterative methods
- New `src/math/math_init.c` — startup-phase CPU feature detection invocation
- New `arch/math/windows-amd64/cpu_features.c` — x86 CPUID-based feature detection (SSE, AVX, AVX-512)
- New `arch/math/linux-amd64/cpu_features.c` — same via inline assembly cpuid
- Remove `tests/math/math_stubs.c` and replace with `src/math/math_scalar.c` in build script
- Existing tests for sqrt, exp, log accuracy go from ~6K failures to ~6K passes
- sin, cos, tanh, sigmoid, silu remain stubbed (return 0.0) — deferred to Chunk 2

## Capabilities

### New Capabilities

<!-- None — extending existing math capability -->

### Modified Capabilities

- `math`: Add public API header, scalar sqrt/exp/log implementations with documented precision bounds, CPU feature detection infrastructure, and arch directory structure with per-platform detection code. Update build scripts to link real implementations instead of stubs.

## Impact

- New source files: `include/fundamental/math/`, `src/math/`, `arch/math/`
- Modifies: `tests/math/build-windows-amd64.bat`, `tests/math/build-linux-amd64.sh` (swap stubs for real sources)
- Depends on: `memory` module (for CPU detection context), existing `startup` framework (Phase 1 init)
- Deferred functions (returning 0.0 via weak stub): sin, cos, tanh, sigmoid, silu, and all vector functions
- No breaking changes
