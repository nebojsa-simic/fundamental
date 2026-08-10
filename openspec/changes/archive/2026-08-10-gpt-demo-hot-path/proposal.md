## Why

The gpt-demo generates at ~0.19 tok/s (64 tokens in ~335s). Profiling shows the hot path is dominated by per-token MoE expert dequantization (24 layers x 4 experts x 3 tensors of 2880x2880 floats dequantized from MXFP4 every token) plus scalar attention loops. Measured optimization attempts:

- A persistent per-expert **f32 disk cache** (100MB/expert, memory-mapped) cannot win on the 16GB target machine: the per-token hot set (96 experts x 100MB = 9.6GB) exceeds the RAM budget the OS can keep resident, so eviction churn made runs 2x *slower* (649s).
- A **q8_0 cache** (25MB/expert) fits the hot set and reached 3.06x (109s), but re-quantizing the weights to q8_0 introduced ~0.4% weight error that flips greedy argmax tokens — the output text no longer matches the baseline.
- The model's **native MXFP4 data is only 4.4MB/expert** and already memory-mapped in the GGUF file. A fused kernel that expands the 4-bit blocks and FMA-accumulates against the activation vector *directly from the mapped MXFP4 data* eliminates the dequant pass entirely: ~1.3GB/token read traffic, ~2-4ms/expert compute, exact numerics (the E2M1 table values and power-of-2 E8M0 scales make every intermediate product exact), and zero cache infrastructure.

## What Changes

- New `fun_math_matrix_vector_mxfp4_f32` primitive: fused dequantize+matrix-vector for MXFP4 blocks — `out[r] = sum_d kvalue(block[..]) * scale * x[d] + b[r]` per row, AVX2-only (no scalar fallback, consistent with all `_f32` functions; the scalar reference lives only in the golden generator).
- gpt-demo attention refactor (kept from the original proposal): `fun_math_rows_dot_f32` score computation, `fun_math_weighted_sum_f32` value accumulation, sink-appended `fun_math_softmax_f32`, hoisted `inv_sqrt_hd` / `rope_mscale` / scratch buffers.
- gpt-demo expert path rewrite: per-expert tensor bases precomputed at layer load from GGUF offsets; the expert loop calls the fused MXFP4 primitive three times per expert — no dequant, no allocations, no name lookups, no cache files.
- Deletion of the entire expert-cache subsystem: `expert_cache.{h,c}`, `arch/expert_cache/*`, cache-only q8 helpers (`q8_quant_row`, `float_to_half`, `q8_matvec`), the on-disk `.bin` cache files, cache stats in `main.c`, and cache fields in `model.h`.
- Housekeeping kept from the original proposal: `rope_mscale` computed once at load; `inv_sqrt_hd` hoisted; per-expert scratch buffers allocated once per forward pass.
- New `demos/gpt-demo/build-linux-amd64.sh` and portable timing via demo-local `arch/timing/<platform>/` (replaces `QueryPerformanceCounter` in `main.c`, removing the `windows.h` dependency).
- No breaking changes to existing public APIs.

## Capabilities

### New Capabilities
- `math`: `fun_math_matrix_vector_mxfp4_f32` — fused AVX2 matrix-vector product over GGUF MXFP4 (E2M1/E8M0) blocks.
- `gpt-demo`: Accelerated inference behavior — vectorized attention via math primitives, fused on-the-fly MXFP4 expert inference with no persistent cache, cross-platform (Windows + Linux) build and timing, greedy output text identical to baseline.

### Modified Capabilities
- `math`: Requirements for two previously-added vector primitives (`rows_dot_f32`, `weighted_sum_f32`) and their use by the gpt-demo.

## Impact

- `include/fundamental/math/math.h` — one new declaration plus MXFP4 block-layout constants (additive).
- `arch/math/avx2/vector.c` — one new AVX2 kernel (no scalar fallback).
- `tests/math/tools/generate_golden.c` + `tests/math/test_data/*` — new golden cases.
- `tests/math/test_vector_accuracy.c`, `tests/math/test_edge_cases.c` — new test sections.
- `demos/gpt-demo/model.c`, `model.h` — expert path rewrite using the new primitive; attention and housekeeping refactor; cache fields removed.
- `demos/gpt-demo/main.c` — portable timing (no `windows.h`), cache stats output removed.
- Deleted: `demos/gpt-demo/expert_cache.{h,c}`, `demos/gpt-demo/arch/expert_cache/`, on-disk `models/*.blk*.exp*.bin` cache files (git-ignored), cache-only helpers in `demos/gpt-demo/q8.{h,c}`.
- New: `demos/gpt-demo/timing.h`, `demos/gpt-demo/build-linux-amd64.sh`, `demos/gpt-demo/arch/timing/{windows-amd64,linux-amd64}/timing.c`.
- `demos/gpt-demo/build-windows-amd64.bat` — drops expert-cache sources, adds the windows-amd64 timing arch file.
