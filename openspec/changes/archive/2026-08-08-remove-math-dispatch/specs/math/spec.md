## ADDED Requirements

### Requirement: ISA units provide vector function implementations
Vector function implementations SHALL be provided by ISA-specific compilation units under `arch/math/<isa>/`. The compilation unit provides strong definitions of the public `fun_math_*` vector functions, selected at link time by which `arch/math/<isa>/` directory is included in the build. No runtime dispatch layer and no function-pointer fallback SHALL exist.

#### Scenario: AVX2 unit linked
- **WHEN** `arch/math/avx2/vector.c` is compiled and linked into the binary
- **THEN** all four vector functions (silu_f32, swiglu_f32, rms_norm_f32, softmax_f32) execute AVX2-optimized code paths without runtime branching

#### Scenario: non-AVX2 build
- **WHEN** the AVX2 unit is not part of the build
- **THEN** the build must provide another ISA unit or fail to link; no implicit scalar fallback exists

### Requirement: Vector-safe vector API
The AVX2 vector implementations SHALL accept unaligned buffers and any `n`. The vectorized main loop SHALL use unaligned loads/stores (`_mm256_loadu_ps`/`_mm256_storeu_ps`), and a scalar tail loop SHALL process the remaining `n % 8` elements.

#### Scenario: unaligned buffers
- **WHEN** the AVX2 code path processes buffers not aligned to 32 bytes
- **THEN** unaligned load/store instructions are used and results remain correct

#### Scenario: tail remainder
- **WHEN** `n` is not a multiple of 8
- **THEN** the remaining `n % 8` elements are processed by the scalar tail loop without out-of-bounds access

#### Scenario: vectorized correctness
- **WHEN** AVX2 results are compared with scalar `fun_math_*` results
- **THEN** each element matches within tolerance (1e-4 relative, 1e-3 absolute) and golden expected values