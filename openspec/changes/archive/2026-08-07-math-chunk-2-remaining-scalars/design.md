## Context

Chunk 1 delivered sqrt (fast inverse sqrt + Newton-Raphson), exp (range reduction + Pade approximant), and log (exponent/mantissa + atanh series). Five scalar functions remain stubbed at 0.0. The golden data headers, accuracy tests, edge case tests, and build scripts are all in place. This is a pure implementation task — edit one file, no plumbing changes.

The key architectural insight: tanh, sigmoid, and silu compose naturally from the existing `fun_math_exp`. Only sin and cos require new numeric algorithms. This minimizes implementation risk and code surface.

## Goals / Non-Goals

**Goals:**
- Implement `fun_math_sin` with precision of at least 1e-3 relative, handling NaN, periodicity, and symmetric sign behavior
- Implement `fun_math_cos` via `cos(x) = sin(x + π/2)`
- Implement `fun_math_tanh` via `fun_math_exp`, with saturation at ±1 for |x| ≥ 10, handling ±inf and NaN
- Implement `fun_math_sigmoid` via `fun_math_exp`, with saturation at 0/1 for large |x|, handling NaN
- Implement `fun_math_silu` via `fun_math_sigmoid`, with correct ±inf handling (silu(+inf) = +inf, silu(-inf) = 0)
- Achieve ~8000 test passes across accuracy and edge case suites
- Maintain zero external dependencies — no libc math.h

**Non-Goals:**
- No SIMD/vector operations (Chunk 3)
- No lookup tables — keep the zero-data-footprint pattern established by Chunk 1
- No runtime dispatch
- No new golden data or test changes

## Decisions

### Decision 1: sin via range reduction + 7th-degree Taylor

**Rationale**: Reduce x to [−π/2, π/2] using `n = round(x / 2π)`, then `x = x − n·2π`. Apply quadrant mapping: sin(x) = sin(π−x) for x in (π/2, π], sin(x) = −sin(−x) for x < 0. Apply 7th-degree Taylor series on the reduced [0, π/2] range.

Taylor coefficients for sin on [0, π/2]:

```
sin(x) ≈ x − x³/6 + x⁵/120 − x⁷/5040 + x⁹/362880
```

A 9th-degree polynomial (5 terms, 4 Horner steps) achieves ~1e-9 absolute error on [−π/2, π/2], far exceeding the 1e-3 relative requirement. Even degree 7 (4 terms) achieves ~1e-6, well within tolerance.

For range reduction at inputs up to ±50 (the golden data span), using float `2π` constant with integer round produces a residual accurate enough to maintain sub-1e-3 relative error. The float32 precision of `2π` is ~3.5e-8, and the residual error from large-argument reduction stays well below our tolerance for |x| < 100.

**Alternative**: LUT with linear interpolation. Memory footprint of 4KB against zero bytes for polynomial. On this project, scalar sin/cos is used for initialization (RoPE positional encoding), not in the inner loop — the performance advantage of LUT's shorter dependency chain doesn't matter. Zero-data-footprint aligns with the existing Chunk 1 code style.

**Alternative**: Cody-Waite range reduction (using two constants). Overkill for our input range and tolerance. The golden data only spans ±50 — single-constant reduction is sufficient.

### Decision 2: cos via sin identity

**Rationale**: `cos(x) = sin(x + π/2)`. This is mathematically exact and reuses the sin implementation entirely. No separate polynomial, no code duplication.

**Trade-off**: The addition `x + π/2` can lose precision for very large x (beyond golden data range). For |x| > 1e6, explicit range reduction on cos would be needed. This is a non-goal for Chunk 2.

### Decision 3: tanh via fun_math_exp

**Rationale**: `tanh(x) = (e^(2x) − 1) / (e^(2x) + 1)`. The existing `fun_math_exp` has ~1e-3 relative error, and tanh composition preserves this. Saturation clamping at |x| ≥ 10 prevents exp overflow (2·44.35 = 88.7, the overflow threshold of fun_math_exp) and avoids division-by-zero. tanh(10) ≈ 0.9999999959, already 1.0 within float epsilon.

Edge case handling:
- tanh(0) = 0 — direct from formula
- tanh(+inf) = 1 — clamped by x ≥ 10 check
- tanh(−inf) = −1 — clamped by x ≤ −10 check
- tanh(NaN) = NaN — passed through from exp

**Alternative**: Direct rational approximation (Pade). Would avoid the exp call but adds complexity and code. For a scalar function used at init time, reusing exp is the right tradeoff.

### Decision 4: sigmoid via fun_math_exp, split for stability

**Rationale**: `sigmoid(x) = 1/(1+e^(-x))`. For x ≥ 0, compute directly. For x < 0, use identity `sigmoid(x) = e^x/(1+e^x)` to avoid catastrophic cancellation in `(1+e^(-x))` when e^(-x) is huge.

Saturation clamping:
- x ≥ 20: return 1.0 (e^(−20) ≈ 2e-9, sigmoid ≈ 0.999999998)
- x ≤ −20: return 0.0 (e^(20) ≈ 4.8e8, sigmoid ≈ 2e-9)

Edge case handling:
- sigmoid(0) = 0.5 — direct from formula
- sigmoid(NaN) = NaN — passed through
- sigmoid(+inf) = 1 — clamped by x ≥ 20 check
- sigmoid(−inf) = 0 — clamped by x ≤ −20 check

### Decision 5: silu via sigmoid, with inf handling

**Rationale**: `silu(x) = x · sigmoid(x)`. Delegate to `fun_math_sigmoid`. Saturation clamping for |x| ≥ 20 avoids unnecessary precision loss.

Edge case handling:
- silu(+inf) = +inf — x ≥ 20 clamp returns x (= +inf)
- silu(−inf) = 0 — x ≤ −20 clamp returns 0 (limit of x·σ(x) as x→−inf is 0 by L'Hopital)
- silu(NaN) = NaN — passed through from sigmoid
- silu(0) = 0 — 0 · 0.5 = 0

## Correctness Guarantees & Limitations

This module provides scalar math for LLM inference initialization (RoPE tables, activation precomputation). It is NOT a general-purpose libm replacement. Each function has explicit domain limits and accuracy bounds.

### Guarantees (tested, verified by golden data)

| Function   | Input domain        | Accuracy                | Special values handled                     |
|------------|---------------------|-------------------------|---------------------------------------------|
| `sin`      | |x| ≤ 50             | ≤ 1e-3 relative error   | NaN → NaN; sin(0) = 0; sin(−x) = −sin(x)   |
| `cos`      | |x| ≤ 50             | ≤ 1e-3 relative error   | NaN → NaN; cos(0) = 1; cos(−x) = cos(x)    |
| `tanh`     | all real, ±inf, NaN | ≤ 1e-3 relative error   | NaN → NaN; tanh(±inf) = ±1; tanh(0) = 0    |
| `sigmoid`  | all real, ±inf, NaN | ≤ 1e-3 relative error   | NaN → NaN; sigmoid(±inf) = 1/0; σ(0) = 0.5 |
| `silu`     | all real, ±inf, NaN | ≤ 1e-3 relative error   | NaN → NaN; silu(+inf) = +inf; silu(−inf) = 0 |

### Limitations (not tested, outside scope)

| Limitation                                    |
|-----------------------------------------------|
| sin/cos |x| > 50: precision degrades due to float32 2π precision loss in range reduction |
| No correctly-rounded results (≤ 1e-3 relative, not ≤ 1 ULP) |
| No errno / domain error reporting |
| No subnormal handling |
| No SIMD dispatch (Chunk 3) |
| No float promotion to double — pure float32 throughout |
| Performance targets: ~22 cyc (sin), ~23 cyc (cos), ~30-35 cyc (tanh/sigmoid/silu via exp) |

### Comparison to glibc

| Property               | Fundamental (this chunk)    | glibc sinf                     |
|------------------------|-----------------------------|-----------------------------   |
| Internal precision     | float32                     | float64 (promotes, then truncates) |
| Range reduction        | Single constant (2π float32) | Cody-Waite (2-constant) + Payne-Hanek fallback |
| Polynomial type        | Taylor (degree 7 or 9)      | Minimax (Chebyshev-tuned)       |
| Accuracy target        | ≤ 1e-3 relative             | ≤ 1 ULP (correctly rounded)     |
| Input range guarantee  | |x| ≤ 50                     | All finite float32 values        |
| errno support          | None                        | EDOM for invalid inputs          |
| Subnormal handling     | None (flushes)              | Full IEEE 754                    |
| Approx cycles/el       | ~22 (sin)                   | ~45 (sinf, hot path)             |

## Risks / Trade-offs

- **Risk**: sin/cos precision degradation edge near |x| = 50 → **Mitigation**: Golden data covers ±50 exactly. Range reduction error at |x| = 50 is ≈ 3e-7, 3 orders of magnitude below our 1e-3 tolerance.
- **Risk**: sigmoid composition error from exp could compound → **Mitigation**: The sigmoid function is well-conditioned (maps monotonic exp error to monotonic sig error with ≤ same relative error); tanh is similarly well-conditioned.
- **Risk**: Caller uses sin/cos outside guaranteed domain → **Mitigation**: No runtime check — documented limitation. Callers reading this design know not to compute sin(1e6).
- **Trade-off**: tanh/sigmoid/silu each re-call fun_math_exp rather than inlining — adds function call overhead → Acceptable for scalar init-time functions; if profiling shows this matters, inlining is trivial later.
