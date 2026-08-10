# Design: gpt-demo Hot-Path Optimizations

## Context

See proposal.md - Why. The gpt-demo forward pass spends most of its ~4.8s/token
dequantizing MXFP4 expert weights (24 layers x 4 experts x 3 tensors x
2880x2880 elements) into f32 heap buffers and running f32 matmuls on them
(~200MB of heap traffic per expert per token). Two cache designs were built and
measured: an f32 disk cache (100MB/expert) whose per-token hot set (9.6GB)
exceeds the 16GB machine's resident capacity, making runs 2x slower via page
churn (649s), and a q8_0 cache (25MB/expert, 3.06x at 109s) whose 0.4% weight
error flips greedy argmax tokens. The winning design reads the model's native
MXFP4 data (4.4MB/expert) directly from the existing GGUF mapping with a fused
kernel. The math module's `_f32` primitives live in `arch/math/avx2/vector.c`
(compiled with `-mavx2 -mfma`, unaligned loads, scalar tails, no scalar
fallback — precedent set by `fun_math_rotary_f32`). The demo currently links
`windows.h` directly for timing; the Linux build script does not exist.

## Goals / Non-Goals

**Goals:**
- One new AVX2 math primitive: fused MXFP4 matrix-vector product, with golden-value tests (scalar reference lives only in the test tool).
- Replace the attention softmax scalar loops with the existing vectorized `fun_math_softmax_f32` plus the two new row/weighted primitives.
- Eliminate per-token expert dequantization by fusing dequant into the matmul over the already-mapped GGUF MXFP4 data — no persistent cache, no disk files, no memory budget.
- Portable demo (no `#ifdef`, no `windows.h` in shared code) using the library's arch-directory + build-script selection pattern.
- Preserve the greedy argmax token sequence (verified against a captured baseline) — including no quantization drift.

**Non-Goals:**
- Scalar fallbacks for the new `_f32` functions (consistent with all existing ones).
- Runtime CPU dispatch (existing precedent).
- A library-level time module (demo-local arch files instead).
- Changing model constants, the tokenizer, or the MoE routing math.
- Vectorizing the router logit loop or the final output projection.
- Any on-disk or in-RAM materialized expert cache (deleted in this change).

## Decisions

### 1. Kernel API Shapes
**Decision:** Three functions in `include/fundamental/math/math.h`, implemented in `arch/math/avx2/vector.c`:

```c
void fun_math_rows_dot_f32(const float *q, const float *x, float *out,
                           size_t n_rows, size_t row_len, size_t row_stride,
                           float scale);
// out[t] = scale * sum_{d<row_len} q[d] * x[t*row_stride + d]

void fun_math_weighted_sum_f32(const float *wgt, const float *x, float *out,
                               size_t n_rows, size_t row_len, size_t row_stride);
// out[d] = sum_{t<n_rows} wgt[t] * x[t*row_stride + d]   (compute-and-store)

void fun_math_matrix_vector_mxfp4_f32(const uint8_t *w, const float *x,
                                      const float *b, float *out,
                                      size_t n_rows, size_t row_len);
// out[r] = sum_{d<row_len} mxp4(w[r], d) * x[d] + b[r]
```

Both `_f32` primitives process the main loop 8 lanes at a time and handle the
tail with scalar code; `fun_math_rows_dot_f32` takes a `scale` parameter so the
demo can fold in `inv_sqrt_hd`; `fun_math_weighted_sum_f32` zeroes `out`
internally. The MXFP4 kernel requires `row_len % 32 == 0` (documented contract;
row_len is 2880 in the demo).

**Alternatives considered:** a single `scale` + accumulate variant of the
weighted sum (rejected: compute-and-store is the only use); a per-head loop of
scalar `fun_math_dot_f32` calls (rejected: ~400K calls/token of call overhead);
a generic "quantized matvec with decode callback" API (rejected: indirection
per block, and the MXFP4 format is fixed by GGUF).

### 2. MXFP4 Fused Kernel: Block Layout and Exactness
**Decision:** The kernel operates directly on GGUF MXFP4 blocks (17 bytes per
32 elements: 1 E8M0 scale byte + 16 nibble bytes). Scale decode mirrors
`gguf_dequant.c`: `scale_f = scale == 0 ? 0.0f : (float)((uint32_t)scale << 23)`
— a pure power of two, so multiplication is exact. The E2M1 kvalues table is
`{0, 0.5, 1, 1.5, 2, 3, 4, 6, -0, -0.5, -1, -1.5, -2, -3, -4, -6}`.

AVX2 path per 32-element block:
1. load 16 nibble bytes;
2. expand both nibbles via `vpshufb` against a ×2-scaled int8 table
   `{0, 1, 2, 3, 4, 6, 8, 12, 0, -1, -2, -3, -4, -6, -8, -12}` — exact (all
   entries are ints) — and compensate by using the scale exponent shifted by
   one (`scale << 23` becomes `(scale-1) << 23`, subnormal powers of two remain
   exact);
3. widen to f32 16 lanes at a time (`cvtepi8_epi16` + `cvtepi16_ps`);
4. broadcast scale, `fmadd` with the activation vector (scale 0 handled by
   skipping the multiply);
5. FMA-accumulate; per row add the f32 bias.

**Rationale:** The dequant values are exact products of small ints/powers of
two, so fused and dequant-then-dot differ only in f32 accumulation order (≤1-2
ulp) — the same class of difference the attention refactor already verified as
text-preserving. This reproduces the baseline's greedy text exactly, which
neither the f32 cache (correct but too slow) nor the q8 cache (fast but
drifting) could.

### 3. Demo Expert Path
**Decision:** `dequant_layer` precomputes, per layer, the GGUF tensor base
offsets for `ffn_gate_exps.weight`, `ffn_up_exps.weight`,
`ffn_down_exps.weight` (MXFP4) and the three f32 bias tensors, plus the
byte-per-expert stride (`w_count * 17 / 32` = 4,406,400) and row stride
(`row_len * 17 / 32` = 1530). The expert loop calls
`fun_math_matrix_vector_mxfp4_f32` three times per selected expert (gate/up
against `hidden`, down against `mid`), with `x`, `b` pointers offset by
`expert * stride`. No cache branch, no online fallback, no per-expert
allocations, no name lookups. Bias pointers are read directly from the f32
GGUF tensors (exact).

**Rationale:** Total steady-state expert cost becomes ~1.3GB/token of read
traffic (all from the already-mapped model file, page-cache resident after the
first tokens) plus ~2-4ms/expert of fused compute — comparable to the warm q8
cache at 109s, without any warm-up, disk, or budget machinery, and with exact
baseline text.

**Alternatives considered:** keep the q8 cache (rejected: greedy text drift);
keep the f32 cache with a larger RAM budget (rejected: 649s measured, thrash
risk); a demo-local `mxfp4.c` kernel (rejected: the primitive is general and
belongs in `fun_math` per the rows_dot/weighted_sum precedent).

### 4. Softmax Fusion with the Sink
**Decision:** Allocate `scores` as `pos+2` floats, write `scores[pos+1] = sink`,
call `fun_math_softmax_f32(scores, pos+2)`, and use `scores[0..pos]` as the
attention weights. The sink's contribution to max and denominator is handled
inside the existing vectorized softmax, deleting the manual max-finding,
subtract, denom-sum, sink-exp, and divide loops.

**Rationale:** Mathematically identical (the sink is just an extra softmax
element; the SWA `-inf` scores contribute `exp(-inf - mx) = 0`). Only float
association differs, so verification is by greedy-argmax equality against the
captured baseline, not bit equality.

### 5. Arch-Directory Pattern Instead of `#ifdef`
**Decision:** Demo-local `arch/` directories, selected by the build scripts,
mirroring the library layout:

```
demos/gpt-demo/
  timing.h                             # double demo_time_now(void)
  arch/timing/windows-amd64/timing.c   # QueryPerformanceCounter
  arch/timing/linux-amd64/timing.c     # clock_gettime(CLOCK_MONOTONIC)
```

The neutral layer exposes one arch hook: `double demo_time_now(void)`.
`main.c` and `model.c` contain no OS code.

**Rationale:** Matches the library's cross-platform convention exactly and is
what enables the Linux build to exist at all. The expert-cache arch layer from
the earlier design is deleted with the cache subsystem.

### 6. Scratch-Buffer Hoisting
**Decision:** The per-expert scratch buffers (`mid`, `gv`, `uv`, `eg`, `dv`)
are allocated once per `model_forward` call and reused across layers/experts.
`inv_sqrt_hd` is hoisted out of the head loop; `rope_mscale`
(`1 + 0.1*log(32)`) is computed once in `model_load` and stored on `Model`.

## Risks / Trade-offs

**[Numerical association]** → softmax fusion, FMA ordering, and the fused
MXFP4 kernel change last-ulp results. Mitigation: tolerance-based tests plus
empirical greedy-argmax equality against the captured baseline. MXFP4 products
are exact (small ints x powers of two), so no quantization drift is introduced.

**[MXFP4 layout dependency]** → the kernel encodes GGUF's MXFP4 block layout
(32 elements / 17 bytes, E2M1 kvalues, E8M0 scale). The layout and table are
documented in `math.h` and mirrored in the golden generator's scalar reference;
a layout change in the library would be caught by the golden tests.

**[AVX2-only availability]** → the new `_f32` functions are only linkable on
AVX2 builds, consistent with every existing `_f32` function. The scalar
reference lives only in `tests/math/tools/generate_golden.c` (no scalar
implementation ships in the library).

**[row_len % 32 != 0]** → undefined per the documented contract; the demo's
row_len (2880) is block-aligned. Tests cover only aligned lengths.

## Migration Plan

Not applicable — additive APIs and a demo refactor verified by output
equality against the baseline capture (`openspec/changes/baseline-output.txt`).
The deleted expert-cache subsystem leaves no migration burden (cache files are
git-ignored and deleted; stale `.bin` files from earlier runs are removed as
part of this change).

## Open Questions

- None.
