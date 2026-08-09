## 1. Library Primitives

- [x] 1.1 Declare `fun_math_rows_dot_f32` and `fun_math_weighted_sum_f32` in `include/fundamental/math/math.h`
- [x] 1.2 Implement AVX2 kernels in `arch/math/avx2/vector.c` (8-lane main loop, scalar tail for `row_len % 8`, unaligned loads)
- [x] 1.3 Verify both kernels handle `n_rows=0` / `row_len=0` without accessing memory

## 2. Golden Values and Tests

- [x] 2.1 Add `gen_vector_rows_dot()` and `gen_vector_weighted_sum()` to `tests/math/tools/generate_golden.c` using scalar reference formulas
- [x] 2.2 Cover row counts {0, 1, 7, 8, 64, 256}, row lengths {1, 7, 8, 9, 63, 64}, and strides `>= row_len`
- [x] 2.3 Build and run the generator to emit `rows_dot_f32_golden.h` and `weighted_sum_f32_golden.h`
- [x] 2.4 Add golden-case tests and LCG-seeded random sweeps vs scalar reference to `tests/math/test_vector_accuracy.c` (tolerance 1e-4/1e-3)
- [x] 2.5 Add edge-case tests (n_rows=0, row_len=0, scale=0) to `tests/math/test_edge_cases.c`
- [x] 2.6 Build and run `tests/math` on Windows and Linux (WSL); all green (683262 passed)

## 3. Demo Attention Refactor

- [x] 3.1 Replace the scalar score loop with `fun_math_rows_dot_f32` (scale = `1/sqrt(hd)`), hoisting `inv_sqrt_hd` out of the head loop
- [x] 3.2 Replace the scalar v-accumulation loop with `fun_math_weighted_sum_f32` and drop the per-layer `attn` zeroing loop
- [x] 3.3 Append the sink logit to `scores` (size `pos+2`) and normalize with a single `fun_math_softmax_f32(scores, pos+2)` call; delete the manual max/sub/denom/divide loops
- [x] 3.4 Compute `rope_mscale` once in `model_load`, store on `Model`, use in `rope_single`
- [x] 3.5 Verify refactored greedy output is identical to the captured baseline (338.56s vs 334.97s baseline, text identical)

## 4. Fused MXFP4 Kernel and Demo Integration

- [ ] 4.1 Declare `fun_math_matrix_vector_mxfp4_f32` and block-layout constants in `include/fundamental/math/math.h`
- [ ] 4.2 Implement the AVX2 kernel in `arch/math/avx2/vector.c` (vpshufb ×2-int8-table expansion with exponent-shifted scale, 16-lane FMA accumulation, scale-0 handling, bias add)
- [ ] 4.3 Add `gen_vector_mxfp4_matvec()` to `tests/math/tools/generate_golden.c` (scalar reference mirrors `gguf_dequant.c` E2M1/E8M0 math); cover block-aligned lengths {32, 128, 2880} x rows {0, 1, 4, 16}, scale-0 blocks, negative/fractional kvalues
- [ ] 4.4 Build and run the generator to emit `mxfp4_matvec_f32_golden.h`; add golden and LCG sweep sections to `tests/math/test_vector_accuracy.c` and edge cases to `tests/math/test_edge_cases.c`
- [ ] 4.5 Build and run `tests/math` on Windows; all green
- [ ] 4.6 Rewrite the demo expert path: `dequant_layer` stores per-layer expert tensor/bias bases and strides; the expert loop calls the fused primitive (gate/up vs hidden, down vs mid)
- [ ] 4.7 Delete `expert_cache.{h,c}`, `arch/expert_cache/`, cache-only q8 helpers (`q8_quant_row`, `float_to_half`, `q8_matvec`), cache fields in `model.h`, cache stats in `main.c`; drop cache sources from `build-windows-amd64.bat`; delete on-disk `models/*.blk*.exp*.bin` files
- [ ] 4.8 Rebuild and run the demo; greedy output text matches the baseline and timing is recorded (target < 150s)

## 5. Cross-Platform Build

- [ ] 5.1 Add `demos/gpt-demo/timing.h` with `double demo_time_now(void)`; add `arch/timing/windows-amd64/timing.c` and `arch/timing/linux-amd64/timing.c`
- [ ] 5.2 Replace `QueryPerformanceCounter` usage in `main.c` with `demo_time_now()` and remove the `windows.h` include
- [ ] 5.3 Update `build-windows-amd64.bat` with the windows-amd64 timing arch file
- [ ] 5.4 Create `demos/gpt-demo/build-linux-amd64.sh` mirroring the Windows build with the linux-amd64 timing arch source
- [ ] 5.5 Build and run the demo on Linux via WSL

## 6. Verification

- [ ] 6.1 Run the demo with the baseline prompt on Windows; greedy token sequence matches the baseline capture
- [ ] 6.2 Run `tests/math` on Linux via WSL; all green
- [ ] 6.3 Run `run-tests-windows-amd64.bat`; all green
- [ ] 6.4 Run `code-format.bat`
- [ ] 6.5 `openspec validate --changes` passes
