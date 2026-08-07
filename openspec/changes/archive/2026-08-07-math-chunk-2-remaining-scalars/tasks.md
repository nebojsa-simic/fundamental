## 1. sin/cos Implementation

- [x] 1.1 Implement `fun_math_sin` — reduce x modulo 2π to [−π/2, π/2] using `round(x / TWOPI)`, apply quadrant sign mapping, compute 9th-degree Taylor polynomial via Horner's method. Handle NaN pass-through.
- [x] 1.2 Implement `fun_math_cos` — delegate to `fun_math_sin(x + PI/2)`.

## 2. tanh Implementation

- [x] 2.1 Implement `fun_math_tanh` — for |x| ≥ 10 clamp to sign(x), otherwise compute `(e^(2x) − 1) / (e^(2x) + 1)` using `fun_math_exp`. Handle NaN/±inf.

## 3. sigmoid Implementation

- [x] 3.1 Implement `fun_math_sigmoid` — for x ≥ 20 return 1.0, for x ≤ −20 return 0.0, for x ≥ 0 compute `1/(1+e^(−x))`, for x < 0 compute `e^x/(1+e^x)` using `fun_math_exp`. Handle NaN.

## 4. silu Implementation

- [x] 4.1 Implement `fun_math_silu` — for |x| ≥ 20 clamp (x for positive, 0 for negative), otherwise compute `x · sigmoid(x)` using `fun_math_sigmoid`. Handle NaN/±inf.

## 5. Verification

- [x] 5.1 Build and run on Windows — all scalar accuracy tests pass (sin 2012, cos 2010, tanh 1011, sigmoid 1015, silu 2015), all edge cases pass (33/33), vector functions still fail (expected for Chunk 3)
- [x] 5.2 Build and run on Linux (Alpine WSL) — scalar accuracy 14115/14115 pass, edge cases 33/33 pass. Identical to Windows results.
- [x] 5.3 Verify scalar tests pass — 14115/14115 scalar accuracy + 33/33 edge cases pass. Vector stubs still fail (Chunk 3). Exit code 1 expected until Chunk 3.
- [x] 5.4 Run performance benchmarks — sin 33.7cyc, cos 27.2cyc, tanh 27.7cyc, sigmoid 33.5cyc, silu 41.8cyc (all < 100cyc/el, within expected range)
- [x] 5.5 Run code formatter (`code-format.bat` / `code-format.sh`) — clang-format applied, build + tests still pass
