## Context

The `math-module-tests` change created a comprehensive test suite with 50K+ golden value cases, but all tests fail because `math_stubs.c` returns 0.0 for every function. The stub file is replaced by real implementations in `src/math/math_scalar.c` and the test build script is updated accordingly.

The library has zero stdlib dependency — no `math.h` from libc. All transcendental functions must be implemented from scratch using polynomial approximations, iterative methods, and bit manipulation. The implementation must also detect CPU features (SSE/AVX/AVX-512) at startup for future SIMD dispatch, even though this chunk only provides scalar code.

Architecture constraint: bare target = latest feature set. `arch/math/windows-amd64/` contains AVX-512 capable code (when we add SIMD in Chunk 3). For now, these directories only hold `cpu_features.c`.

## Goals / Non-Goals

**Goals:**
- Implement `fun_math_sqrt(float)` with precision of at least 1e-5 relative, handling ±0.0 and NaN
- Implement `fun_math_exp(float)` with precision of at least 1e-3 relative, handling ±inf, NaN, overflow, underflow
- Implement `fun_math_log(float)` with precision of at least 1e-3 relative, handling 0, ±inf, NaN, negative inputs
- Create `include/fundamental/math/math.h` with the public API surface (all 8 scalar + 4 vector declarations)
- Implement CPU feature detection via CPUID (Windows) and inline asm (Linux)
- Replace `math_stubs.c` with `src/math/math_scalar.c` in the test build
- Achieve ~6,000 test passes for sqrt, exp, log (accuracy + edge cases)

**Non-Goals:**
- No SIMD/vector operations (Chunk 3)
- No sin, cos, tanh, sigmoid, silu (Chunk 2) — these remain stubbed at 0.0
- No runtime dispatch (function pointer tables) — deferred until both scalar and SIMD exist
- No `-mavx512f` in build flags (no SIMD yet) — build script uses plain gcc

## Decisions

### Decision 1: sqrt via fast inverse sqrt + Newton-Raphson

**Rationale**: The Quake III fast inverse sqrt algorithm (`0x5f3759df` magic constant) provides an excellent initial guess for `1/sqrt(x)`. One Newton iteration refines it to ~1e-5 precision. Multiply by x to get sqrt(x). This avoids the cost of a general-purpose iterative solver and works with integer bit manipulation only — no libc dependency.

**Alternative**: Bailey's method (iterative multiplication). Rejected — slower convergence.

### Decision 2: exp via range reduction + Pade (4,4) approximant

**Rationale**: `exp(x) = 2^(x / ln(2))`. Split into integer and fractional parts: `2^n * exp2(frac)`. The integer n is handled by direct float bit manipulation (exponent field). The fractional part [-0.5, 0.5] uses a Pade rational approximation (ratio of two 4th-degree polynomials) that achieves 1e-6 relative error with only 15 operations.

**Alternative**: Taylor series (slow convergence), lookup table (memory cost). Pade gives the best precision-per-operation ratio for the target range.

### Decision 3: log via Newton iteration seeded by frexp-like bit extraction

**Rationale**: Extract exponent field of the float to get `n = floor(log2(x))`. The mantissa gives `m in [1, 2)`. Then `log(x) = n * ln(2) + log(m)`. For `log(m)` where m is in [1,2), use Newton iteration on `exp(y) = m` starting from a polynomial seed. This converges to 1e-6 in 2-3 iterations.

**Alternative**: Pade approximant directly on log. Rejected — requires separate handling per sub-interval for good precision.

### Decision 4: CPU detection via CPUID, stored in static global

**Rationale**: `__cpuid` intrinsic on Windows, inline asm `cpuid` on Linux. Results stored in a file-scope static struct (`g_cpu_features`). No heap allocation. Accessed via `fun_math_has_avx2()` etc. The detection runs at startup Phase 1 (`fun_math_init()` called from `fun_startup_run`).

### Decision 5: Unimplemented functions remain weak stubs in math_scalar.c

**Rationale**: Rather than keeping a separate `math_stubs.c`, the scalar file provides weak `__attribute__((weak))` implementations for sin/cos/tanh/sigmoid/silu that return 0.0. Chunk 2 replaces these with strong implementations. This keeps the build script simple — always compile `math_scalar.c`, never need to swap files.

**Alternative**: Keep separate stubs file and modify build script each chunk. Rejected — more build script churn.

## Risks / Trade-offs

- **Risk**: Pade (4,4) for exp may need (5,5) for 1e-4 relative precision across full range → **Mitigation**: Test against golden values; upgrade to (5,5) if failures exceed tolerance
- **Risk**: Fast inverse sqrt constant `0x5f3759df` is tuned for `float` — double-precision variant may differ → **Mitigation**: We only target `float` (single precision); the constant is known-good for IEEE 754 single precision
- **Trade-off**: Weak symbols pollute the symbol table with zero-return stubs → Acceptable trade-off for simpler build script management across chunks
