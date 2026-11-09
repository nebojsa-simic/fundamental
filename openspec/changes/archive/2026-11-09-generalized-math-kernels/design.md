# Design

## Context

The `math` module ships scalar transcendental kernels (`src/math/math_scalar.c`) and ISA-specific vector kernels under `arch/math/{scalar,avx2,avx512}/`. ISA selection is link-time: the AVX2 unit defines its kernels as weak symbols, and the scalar unit defines strong symbols, so a build resolves each `fun_math_*` function to the strongest unit it links. There is no runtime dispatch and none should be introduced for CPU ISA selection (see the cleaned `math` spec).

There is currently no uniform seam for describing an operation and routing it to a backend. The `compute` module schedules task graphs but has no concept of a math backend or tensor descriptors.

## Goals / Non-Goals

**Goals:**
- Introduce a pluggable backend (function-pointer vtable) with a registry so callers can route math operations to a backend, enabling a future GPU/accelerator backend without changing call sites.
- Introduce a generic op dispatch (op code + dtype + strided tensor views) for elementwise unary/binary and single-input reductions.
- Expose the heterogeneous structured kernels (matrix-vector, dot, rms_norm, softmax, rotary) through the same backend contract.
- Preserve the existing link-time weak-symbol ISA dispatch and the per-arch build scripts exactly as they are.

**Non-Goals:**
- No GPU/CUDA/Vulkan/Metal backend code (interface + contract only).
- No f16/f64 kernel implementations (dtype reserved).
- No SIMD fast-paths for the *generic* dispatch path or the new add/mul/sum/max kernels in this change.
- No changes to existing `fun_math_*` signatures or to the per-arch build scripts.

## Decisions

### D1: Separate header `math_kernels.h`; new kernels go in `math.h`
The dispatch/backend types (`FunMathDtype`, `FunMathOp`, `FunMathTensor`, `FunMathBackend`) and dispatch functions live in a new `include/fundamental/math/math_kernels.h` so `math.h` stays focused on concrete kernels. The four new portable kernels (`add`/`mul`/`sum`/`max`) follow the `fun_math_*_f32` convention and are declared in `math.h` alongside `dot_f32`.
- *Alternative considered*: put everything in `math.h`. Rejected — mixes a dispatch abstraction into a kernel header and forces all consumers to pull in backend types.

### D2: Default CPU backend is a static vtable pointing at the built-in kernels
`fun_math_backend_get()` returns a pointer to a static `FunMathBackend` whose unary/binary/reduce entries are generic strided-loop implementations in `src/math/math_kernels.c`, and whose structured entries point at the existing `fun_math_*` kernels. `fun_math_backend_set(NULL)` restores this default. No heap allocation; a file-scope pointer holds the current backend. The pointer is process-global and mutable only through `fun_math_backend_set`, which callers are expected to invoke during single-threaded setup before any concurrent dispatch (documented in the header).
- *Alternative considered*: resolve the default backend lazily via `fun_math_init`. Rejected — the default is compile-time constant and needs no init.
- *Alternative considered*: guard the pointer with a mutex. Rejected for v1 — backend selection is a startup-time concern; a lock would add cost to every dispatch.

### D3: Generic dispatch is a strided loop; new kernels are the contiguous case
The default CPU backend's `unary`/`binary`/`reduce` entries implement strided elementwise loops over the tensor descriptors. `unary` applies the scalar `fun_math_*` function per element. `binary`/`reduce` apply inline arithmetic per element; when all tensors are contiguous (stride 1) they delegate to the public `fun_math_add_f32`/`fun_math_mul_f32`/`fun_math_sum_f32`/`fun_math_max_f32` kernels. This keeps v1 simple and correct for every stride and op.
- *Alternative considered*: forward contiguous unary ops to the existing `fun_math_exp_f32`/`fun_math_silu_f32` batch kernels for SIMD. Rejected for v1 — it adds per-op divergence (no batch kernel exists for sqrt/tanh/sigmoid) and is a pure optimization that can be layered on later without API change.

### D4: Structured kernels exposed as nullable vtable entries + dispatch wrappers
`FunMathBackend` carries typed function pointers for matrix-vector, dot, rms_norm, softmax, and rotary. A dispatch wrapper `fun_math_<name>_dispatch(...)` (for example `fun_math_matrix_vector_f32_dispatch`) calls the backend entry when non-NULL, otherwise the built-in kernel. This gives a GPU backend a single, uniform place to override each structured op while keeping the existing `fun_math_*` symbols unchanged (so the per-arch weak-symbol dispatch for those kernels is untouched).
- *Alternative considered*: fold the structured ops into the generic `unary`/`binary`/`reduce` via a union parameter blob. Rejected — their signatures are heterogeneous and forcing them into one signature would be lossy and harder to read.

### D5: `dot` is a structured entry, not a generic reduction
`dot` takes two inputs, which does not fit the single-input `fun_math_reduce(op, dtype, in, out)` signature. It stays the existing `fun_math_dot_f32` kernel and is routed through the backend as a structured entry (`fun_math_dot_f32_dispatch`). The generic `reduce` covers sum and max only.

### D6: Stride is measured in elements, not bytes
`FunMathTensor.stride` is an element stride (1 = contiguous), matching how the existing `rows_dot_f32`/`weighted_sum_f32` treat `row_stride`. Element strides keep the descriptor independent of dtype size and avoid byte-width bugs when f16/f64 are added.

## Risks / Trade-offs

- [Generic path is scalar in v1] → Acceptable; the primary purpose is the dispatch seam and offload contract, not peak CPU throughput. SIMD fast-paths can be added to the default backend later without API change.
- [Two headers to remember] → Documented in `math_kernels.h`'s header comment and cross-linked from `math.h`.
- [Backend vtable ABI stability] → This is a C library; the vtable layout is public. Adding new entries must append to the end of the struct to preserve source compatibility for backend implementers.
- [No validation of reserved dtypes] → Calling a dispatch with `F16`/`F64` is a no-op in v1 (the output is left unmodified). A future change can add real behavior without breaking the enum.
- [Backend pointer is not thread-safe] → `fun_math_backend_set` must be called during single-threaded setup, not while dispatches are in flight. Documented in the header; acceptable because backend choice is a startup decision.

## Migration Plan

Additive only: new header, new source file, new test source wired into `tests/math` build scripts. No existing symbols change, so there is no rollback beyond removing the new files. Existing callers of `fun_math_*` are unaffected.

## Open Questions

None.
