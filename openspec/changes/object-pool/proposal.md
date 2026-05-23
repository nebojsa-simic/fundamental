## Why

`fun_memory_allocate` always invokes `mmap` syscall — fine for large/rare allocations, prohibitive for frequent small-object create/destroy cycles. Glibc/ptmalloc solves this with slab allocators and free lists that serve requests without entering the kernel. Fundamental should offer an object pool module: pre-allocate a batch of same-sized slots once, then acquire/release them in O(1) without syscalls. Eliminates allocation pressure on hot paths.

## What Changes

- New `object-pool` module with fixed-size, fixed-capacity slot pools
- `fun_object_pool_create(elementSize, capacity)` — allocates slab once, builds intrusive free list
- `fun_object_pool_acquire(pool)` — pops a slot from free list, O(1), no syscall, returns NULL when exhausted
- `fun_object_pool_release(pool, slot)` — validates ownership, pushes slot back to free list, O(1), no syscall
- `fun_object_pool_destroy(pool)` — frees slab, detects outstanding (unreleased) slots
- `DEFINE_OBJECT_POOL_TYPE(T)` macro generates type-safe pool struct and typed acquire/release functions
- Intrusive free list: next-pointer stored in the slots themselves, zero per-slot metadata
- No auto-growth, no thread safety — single slab, single-threaded, sized at creation time

## Capabilities

### New Capabilities
- `object-pool`: Fixed-size, fixed-capacity object pool with pre-allocation, O(1) acquire/release, leak detection on destroy, and type-safe macro wrapper.

### Modified Capabilities
<!-- None — purely additive module -->

## Impact

- New source files: `src/object-pool/`, `include/fundamental/object-pool/`, `tests/object-pool/`
- Depends on: `memory` module (`fun_memory_allocate`, `fun_memory_free`)
- No breaking changes to existing APIs
- Cross-platform (all logic in `src/`, no `arch/` code — memory module handles platform abstraction)
