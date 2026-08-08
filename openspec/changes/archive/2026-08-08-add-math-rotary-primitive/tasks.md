## 1. Library Primitive

- [x] 1.1 Declare `fun_math_rotary_f32` in `include/fundamental/math/math.h`
- [x] 1.2 Implement AVX2 kernel in `arch/math/avx2/vector.c` (head loop, 8-lane rotation, in-place safe)
- [x] 1.3 Add scalar tail loop for `half % 8 != 0`

## 2. Golden Values

- [x] 2.1 Add `gen_vector_rotary()` to `tests/math/tools/generate_golden.c` using the scalar reference formula
- [x] 2.2 Cover half values {1, 7, 8, 9, 31, 32, 33, 64} and n_heads {1, 3, 8, 64}
- [x] 2.3 Build and run the generator to emit `tests/math/test_data/rotary_f32_golden.h`

## 3. Tests

- [x] 3.1 Add golden-case test to `tests/math/test_vector_accuracy.c` (tolerance 1e-4/1e-3)
- [x] 3.2 Add LCG-seeded random sweep vs scalar reference including in-place aliasing
- [x] 3.3 Add edge-case tests (n_heads=0, half=0, half=1) to `tests/math/test_edge_cases.c`
- [x] 3.4 Build and run `tests/math` on Windows; all green

## 4. Demo Refactor

- [x] 4.1 Add `float *rope_pre` to `Model` in `demos/gpt-demo/model.h`
- [x] 4.2 Precompute `rope_pre[half]` in `model_load` (geometric series + ramp factor)
- [x] 4.3 Free `rope_pre` in `model_free`
- [x] 4.4 Rewrite `rope_single`: the_arr once, one cos/sin pair, two `fun_math_rotary_f32` calls
- [x] 4.5 Rebuild demo and verify greedy argmax output unchanged from pre-refactor binary

## 5. Verification

- [x] 5.1 Run `code-format.bat`
- [x] 5.2 `openspec validate --changes` passes
