## Context

The Fundamental Library has no math module — no transcendental functions, no vector operations, no floating-point infrastructure beyond basic arithmetic. The LLM execution engine requires sqrt, exp, log, sin, cos, tanh, sigmoid, silu, RMS norm, softmax, and SwiGLU. These are called billions of times per inference and must be correct and fast.

This phase is **tests only**. No implementation code is written. The tests define the API contract, accuracy bounds, performance targets, and edge-case behavior. Implementation follows in Phase 2.

Constraints inherited from the library:
- Zero stdlib: no `math.h`, no `printf`, no `assert.h`, no `stdlib.h`
- No `#ifdef` in library code — platform selection via build script source file inclusion
- `-Os` optimization only — SIMD must be hand-written intrinsics, not compiler auto-vectorization
- Bare arch target = latest feature set (e.g., `windows-amd64/` = AVX-512, not `-avx512` suffix)
- All tests self-contained: golden values baked into C headers, string output via `fun_console_write_line`

## Goals / Non-Goals

**Goals:**
- Establish the public API contract for `fun_math_*` functions (names, signatures, error bounds)
- Measure performance baseline (RDTSC cycle counts) before implementation exists
- Prove the golden-value testing pattern (offline Python generation → C header → test)
- Prove the cross-consistency pattern (scalar reference vs. SIMD in same binary)
- Validate the arch/ directory convention with build scripts per feature set
- Cover scalar single-element functions: sqrt, exp, log, sin, cos, tanh, sigmoid, silu
- Cover vector batch functions: silu_f32, rms_norm_f32, swiglu_f32, softmax_f32

**Non-Goals:**
- No actual implementation of math functions (tests compile against empty stubs or scalar reference)
- No older feature-set builds (avx2, sse2, separate neon build) — deferred
- No LUT implementations (only scalar + latest SIMD paths)
- No thread-parallel tests
- No integration with LLM module (that requires tensor module first)
- No arithm (fabs, floor, ceil, max, min, round) — those are trivial and can be added later
- No fused exp+max for softmax optimization — deferred to implementation phase

## Decisions

### Decision 1: Golden values generated offline by Python script

**Rationale**: Without `math.h` or libc, we cannot compute reference values at test runtime. A Python script (`tests/math/tools/generate_golden.py`) uses Python's `math` module (IEEE 754 compliant) to generate C headers containing input/expected/tolerance triplets. The script runs once offline; the headers are committed to the repo.

**Alternatives considered**:
- Compute reference at compile time via C preprocessor macros: rejected — preprocessor cannot do float math
- Hard-code golden values by hand: rejected — error-prone, doesn't scale to thousands of test cases
- Bypass test accuracy entirely until Phase 2: rejected — defeats the purpose of tests-first

### Decision 2: Pseudo-random values via LCG, not libc rand()

**Rationale**: The consistency tests need pseudo-random inputs but cannot use `rand()`. A simple LCG (`rng = rng * 1103515245 + 12345`) provides deterministic, reproducible sequences across platforms. The seed is fixed per test run for reproducibility.

### Decision 3: Performance measurement via RDTSC inline assembly

**Rationale**: No `clock()` or `gettimeofday()` available. RDTSC reads the CPU timestamp counter directly via inline assembly (`rdtsc` instruction on x86, `PMCCNTR_EL0` on ARM). This is the lowest-overhead timing primitive available without OS syscalls.

**Trade-off**: RDTSC counts cycles, not wall time. Results vary with CPU frequency scaling and turbo boost. Mitigation: run tests pinned to a specific core where possible; report relative speedups rather than absolute latency.

### Decision 4: Test output via fun_console_write_line with manual int-to-string

**Rationale**: No `printf`. Test results (pass/fail counts, cycle counts) are formatted using existing `fun_string_from_int` and `fun_string_from_double` from the string module, then written via `fun_console_write_line`.

### Decision 5: Scalar fallback compiled alongside SIMD in same binary

**Rationale**: The consistency test compares SIMD output against scalar output on identical inputs. Both implementations exist in the same binary — the SIMD vector function calls `_math_scalar_*_tail` for remainder elements, and the test calls the same tail function on the full array to produce reference output. No weak linking, no runtime dispatch — just two functions in the same binary.

### Decision 6: Arch directory convention — bare target = latest feature set

**Rationale**: `arch/math/windows-amd64/` contains AVX-512 code. Older feature sets (avx2, sse2) would go in `arch/math/windows-amd64-avx2/` etc. This matches the user's preference: the default build targets the newest hardware, and fallbacks are explicitly suffixed. Only the bare target is implemented in this phase.

### Decision 7: Public API header not created yet

**Rationale**: The tests define the API contract implicitly through function declarations in the test files. The actual `include/fundamental/math/math.h` header is created in Phase 2 (implementation). The test files contain forward declarations of the functions they test, which must match what Phase 2 eventually provides.

**Alternative**: Create the header now with empty implementations. Rejected — this creates a file that can't compile until Phase 2, and the test build would need stub `.c` files. Instead, test-only declarations keep the build self-contained.

## Risks / Trade-offs

- **Risk**: Golden values generated with Python 64-bit floats may differ from C 32-bit float rounding → **Mitigation**: Python script rounds inputs to `float32` precision before computing expected outputs; tolerance accounts for single-precision rounding
- **Risk**: RDTSC values are noisy on virtualized/cloud CPUs → **Mitigation**: Run enough iterations (100K+) to amortize noise; report median or min rather than mean
- **Risk**: Test compiles but can't link (no implementation) → **Mitigation**: Test files contain stub function bodies that return sentinel values; accuracy tests always fail with stubs, verifying the test harness works before Phase 2
- **Trade-off**: Testing without an implementation means the test harness itself is untestable → **Mitigation**: Include deliberately incorrect golden values in one edge-case test to verify the harness detects failures correctly
