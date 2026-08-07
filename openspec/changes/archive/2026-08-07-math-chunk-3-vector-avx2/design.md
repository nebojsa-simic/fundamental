## Context

Chunks 1+2 delivered all 8 scalar functions with ~1e-3 relative accuracy. Four vector functions remain stubbed. The LLM inference hot path applies silu, rms_norm, swiglu, and softmax over millions of float32 elements per forward pass. Scalar loops are unusably slow for this workload.

Machine: AMD Ryzen 7 5800H (Zen 3) — AVX2 + FMA, no AVX-512F.

## Goals / Non-Goals

**Goals:**
- Implement AVX2 (8-wide) versions of all 4 vector functions
- Runtime dispatch: AVX2 path when supported, scalar fallback otherwise
- Aligned-only API — no tail branches in SIMD code
- Reusable AVX2 helpers: exp, sigmoid, silu
- 100% test pass rate on golden data and consistency tests
- Exit code 0

**Non-Goals:**
- SSE2 implementations
- AVX-512F (machine doesn't support it)
- Tail handling in SIMD code (aligned-only API)
- Alignment validation at runtime

## Decisions

### Decision 1: Aligned-only API — no tail handling

Caller guarantees 32-byte aligned buffers and n%8==0 on AVX2 path. SIMD code uses aligned loads and pure 8-wide loops. Scalar fallback handles arbitrary n.

### Decision 2: Dispatch via function pointer table

Each function has a `typedef` and a file-scope static pointer initialized to the scalar implementation. `fun_math_init()` sets pointers to AVX2 versions after CPU detection. Public entry points delegate through the pointer.

### Decision 3: Internal AVX2 helpers in vector.c

- `_mm256_exp_ps` — Pade (4,4) approximation, same algorithm as scalar `fun_math_exp` vectorized via `_mm256_fmadd_ps`
- `_mm256_sigmoid_ps` — 1/(1+exp(-x)), clamping ±20, split for stability
- `_mm256_silu_ps` — x * sigmoid(x)

### Decision 4: Scalar fallbacks in dispatch.c

Simple loops calling existing scalar functions or inline scalar implementations.

### Decision 5: Shared AVX2 file across platforms

`arch/math/avx2/vector.c` is pure intrinsics — no OS dependencies. Both platform build scripts compile this same file with `-mavx2 -mfma`.

## Algorithms (AVX2, 8-wide, no tails)

### _mm256_exp_ps(__m256 x)

Range reduce via `_mm256_round_ps(x * L2E + 0.5)` → integer k, residual `r = x - k*LN2` via FMA. Pade (4,4) evaluated with Horner's method via `_mm256_fmadd_ps`. Reconstruct: add k+127 to exponent field via bit shift + integer add on float bits.

### silu_f32

Load 8 → `_mm256_silu_ps` → store 8. Repeat n/8 times.

### swiglu_f32

Load 8 gate + 8 up → `gate * _mm256_silu_ps(gate) * up` → store 8.

### rms_norm_f32

Pass 1: accumulate sum-of-squares via `_mm256_fmadd_ps`, horizontal sum via hadd chain → scalar ss. Compute `scale = 1.0f / fun_math_sqrt(ss/n + eps)`. Pass 2: `x[i] * scale * weight[i]` 8-wide.

### softmax_f32

In-place. Pass 1: horizontal max via `_mm256_max_ps` → scalar m. Pass 2: `_mm256_exp_ps(x-m)`, accumulate sum. Pass 3: divide each by sum, 8-wide.

## Correctness Guarantees & Limitations

| Function | Tolerance (golden) | Consistency (AVX2 vs scalar) |
|----------|-------------------|------------------------------|
| silu_f32 | ≤ 1e-4 abs / 1e-3 rel | ≤ 1e-5 abs |
| rms_norm_f32 | ≤ 1e-4 abs / 1e-3 rel | ≤ 5e-4 abs / 5e-3 rel |
| swiglu_f32 | ≤ 1e-4 abs / 1e-3 rel | — (no consistency test) |
| softmax_f32 | ≤ 1e-4 abs / 1e-3 rel | — (no consistency test) |

| Limitation |
|------------|
| AVX2 path: buffers must be 32-byte aligned, n%8==0 |
| AVX2 and scalar use different exp evaluation → results differ at ~1e-6 level |
| No SSE2 or AVX-512F (machine limitation) |
| No alignment validation at runtime |

## Risks / Trade-offs

- **Risk**: AVX2 Pade exp precision differs from scalar Pade → consistency test failures → **Mitigation**: Verify during implementation; widen tolerance if needed.
- **Risk**: softmax max reduction precision differs from scalar → sum check fails → **Mitigation**: Same algorithm, same input order — should agree within tolerance.
- **Trade-off**: Aligned-only API limits callers → Documented. LLM inference allocates aligned tensors. Others use scalar fallback.
