# Proposal

## Why

The `math` module exposes a flat set of ad-hoc `fun_math_*` kernels that callers invoke directly. There is no uniform way to describe an operation and route it to a backend, so a caller that wants to run the same op on the CPU today and offload it to a GPU (or another accelerator) later must rewrite every call site. A backend dispatch layer — a pluggable backend vtable plus a generic op dispatch over dtype-tagged, strided tensors — closes that seam while leaving the existing link-time ISA selection untouched.

## What Changes

- Add a backend abstraction for the `math` module: a registry (`fun_math_backend_set` / `fun_math_backend_get`) holding a vtable of kernel function pointers, defaulting to the built-in CPU backend that forwards to the existing `fun_math_*` weak-symbol kernels.
- Add a dtype enum (`FunMathDtype`) with `F32` implemented and `F16`/`F64` reserved, and an op enum (`FunMathOp`) covering unary, binary elementwise, and single-input reduction (sum, max) operations.
- Add a strided tensor descriptor (`FunMathTensor`) and generic dispatch entry points (`fun_math_unary` / `fun_math_binary` / `fun_math_reduce`) that route an op+dtype+tensors triple to the current backend.
- Route the existing structured kernels (`matrix_vector_f32`, `dot_f32`, `rms_norm_f32`, `softmax_f32`, `rotary_f32`) through the backend as explicit, offloadable vtable entries with dispatch wrappers that fall back to the built-in kernel.
- Add portable scalar implementations for the missing elementwise binary (`add`, `mul`) and reduction (`sum`, `max`) kernels in `src/math/math_scalar.c` so the generic dispatch has real work to do (no SIMD versions in this change).
- Document the backend interface as the contract a future GPU/accelerator backend must implement; no GPU code is written in this change.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `math`: add a backend dispatch/registry layer and a generic op dispatch (op code + dtype + strided tensor views) over the existing kernels, and add the missing elementwise binary and reduction kernels.

## Impact

- New public header `include/fundamental/math/math_kernels.h` (backend, dtype, op, tensor, dispatch declarations); `include/fundamental/math/math.h` gains the new elementwise/reduction kernel declarations.
- New source `src/math/math_kernels.c` (registry + dispatch + default CPU backend); the four portable kernels are added to `src/math/math_scalar.c`.
- `tests/math/` gains a dispatch test source wired into the existing build scripts. The per-arch weak-symbol ISA dispatch mechanism is untouched; build scripts change only to add the new source files.
- No breaking changes: existing `fun_math_*` signatures and call sites are unchanged.
