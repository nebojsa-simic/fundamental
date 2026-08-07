## Why

The current math Chunk 3 implementation uses a runtime dispatch table (function pointers set during `fun_math_init()`) to choose between AVX2 and scalar vector function implementations. This adds unnecessary complexity: typedefs, function pointer storage, dispatch init, and per-call indirect branches.

The correct architecture treats ISAs like platform architectures: one compilation unit per ISA, compiled with its own flags. The ISA compilation unit provides the strong public function definitions directly; no dispatch layer, no function pointers, no CPU detection dependency for implementation selection, and no scalar fallback. The build selects the implementation by including the appropriate `arch/math/<isa>/` directory. Link-time resolution against the public `fun_math_*` symbols is the only selection mechanism.

## What Changes

- DELETE `src/math/math_dispatch.c` — entire function pointer dispatch layer
- MODIFY `arch/math/avx2/vector.c` — rename internal `_silu_f32_avx2` etc. to public `fun_math_silu_f32` etc. (strong symbols); correct sigmoid blend; use unaligned loads/stores with scalar tails for any `n`; `always_inline` internal vector helpers so GCC/MinGW at `-Os` does not emit 32-byte-aligned stack spills (VMOVAPS) against a 16-byte-guaranteed RSP
- MODIFY `src/math/math_init.c` — remove `_math_dispatch_init()` call
- MODIFY `arch/math/windows-amd64/cpu_features.c` — remove `_math_dispatch_init` forward declaration
- MODIFY `tests/math/build-windows-amd64.bat` — remove `math_dispatch.c` from link
- MODIFY `tests/math/build-linux-amd64.sh` — same
- MODIFY `demos/gpt-demo/build-windows-amd64.bat` — same

## Capabilities

### Modified Capabilities

- `math`: Vector functions are provided by the ISA compilation unit (e.g. `arch/math/avx2/vector.c`) as strong public symbols, selected by which `arch/math/<isa>/` directory is part of the build. No runtime dispatch is used. The vectorized API accepts any `n` and any buffer alignment (unaligned loads/stores plus scalar tail).

## Impact

- Deletes 1 file, modifies 7 files
- No API changes — public function signatures unchanged
- No header changes
- Build script changes are mechanical (drop `math_dispatch.c` from link)
- Vector functions now work for unaligned buffers and any `n` (previously aligned-only, multiples-of-8-only)