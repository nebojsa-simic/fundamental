## Context

Fundamental's memory module uses per-allocation `mmap`/`munmap` syscalls. For hot paths that repeatedly create and destroy same-sized objects (connection handles, request structs, buffer nodes), every cycle hits the kernel. Object pools eliminate this: pre-allocate a slab of N same-sized slots once via `fun_memory_allocate`, then serve acquire/release from a free list — no syscalls on the hot path.

The pool is sized once at creation time and never grows. Exhaustion is an explicit error. Caller manages threading.

## Goals / Non-Goals

**Goals:**
- O(1) acquire/release without syscalls
- Intrusive free list (next-pointer stored in each free slot, zero per-slot overhead)
- Fixed capacity, pre-allocated once at create time
- Leak detection on destroy (report outstanding slots)
- `DEFINE_OBJECT_POOL_TYPE(T)` macro for type-safe pools
- Cross-platform (no `arch/` code, memory module handles platform)

**Non-Goals:**
- Thread safety (caller manages synchronization externally)
- Auto-growth or elastic capacity (pool is fixed size, permanently)
- Variable-size slots per pool (each pool has one element size)
- Defragmentation or compaction
- Guaranteed alignment beyond what `fun_memory_allocate` provides (page-aligned)

## Decisions

### Decision 1: Intrusive free list (pointer stored in slot)

Free slots form a singly-linked list. When a slot is free, its first `sizeof(void*)` bytes contain the address of the next free slot. When acquired, the caller can overwrite those bytes normally. This means zero per-slot metadata — no separate bitmap, no index array.

**Constraint**: `elementSize >= sizeof(void*)` (8 bytes on amd64). Smaller elements would need a separate free index scheme — out of scope.

**Alternatives considered:**
- External free-index array: 8 bytes per slot overhead (pointer to each slot). Scales linearly with capacity. Rejected — intrusive is standard and zero-overhead.
- Bitmap + bit-scan: O(1) with BSF but requires bitmap storage (capacity/8 bytes). Overkill. Rejected.

### Decision 2: Single slab, fixed capacity

One `fun_memory_allocate` call at create time allocates `elementSize * capacity` bytes. Free list is built by iterating over slots and linking them. No further syscalls. Pool struct is small, no slab-list management.

**Pool struct layout:**
```
ObjectPool {
    size_t elementSize;
    size_t capacity;
    size_t freeCount;
    void  *freeList;     // head of intrusive free list
    void  *memory;       // fun_memory_allocate result (for destroy/free)
}
```

### Decision 3: Release validation via address range

`release(pool, slot)` validates ownership by checking `slot >= memory && slot < memory + elementSize * capacity`. O(1), single comparison. No per-slot owner pointer needed. No false positives with page-aligned slabs from mmap.

### Decision 4: Type-safe macro

Following `DEFINE_ARRAY_TYPE(int)` pattern from collections:

```c
DEFINE_OBJECT_POOL_TYPE(MyStruct)
// Generates:
//   typedef struct { ObjectPool pool; } MyStructPool;
//   MyStruct *fun_object_pool_MyStruct_acquire(MyStructPool *p);
//   CanReturnError(void) fun_object_pool_MyStruct_release(MyStructPool *p, MyStruct *s);
//   CanReturnError(void) fun_object_pool_MyStruct_destroy(MyStructPool *p);
//   size_t fun_object_pool_MyStruct_free_count(MyStructPool *p);
//   size_t fun_object_pool_MyStruct_capacity(MyStructPool *p);
```

Raw `void*` functions (`fun_object_pool_acquire`, etc.) also exposed for callers who prefer untyped pools.

## Risks / Trade-offs

- **Slot size minimum (8 bytes)**: slots < 8 bytes unsupported because they can't hold the free-list next pointer. Mitigation: document; bitmap variant can be added later. Typical use cases (structs, handles) are well above 8 bytes.
- **No double-release detection**: releasing an already-free slot corrupts the free list (circular linkage possible). Mitigation: document as caller responsibility; ownership check just verifies pool membership, not acquired/free state.
- **Fixed capacity planning**: callers must size the pool correctly for their workload. Mitigation: document sizing guidance; exhaustion is a hard error that callers handle.
