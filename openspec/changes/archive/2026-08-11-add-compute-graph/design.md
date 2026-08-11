# Design

## Context

The gpt-demo performs single-threaded LLM inference on a GPT-OSS-20B MoE model. The forward pass per token iterates 24 layers, each consisting of attention (Q/K/V matvecs, RoPE, KV cache, attention scores, output projection, residual) followed by an MoE FFN block (router, 4 expert projections with SiLU gating, weighted accumulation). Measured speed is 0.45 tok/s (~2.2s/token) on the target machine.

The expert block dominates runtime (~78% of compute). Each expert runs three independent MXFP4 matrix-vector products (gate, up, down) against the hidden state and is fully independent of the other three experts — 4-way parallel work across 24 layers, totaling 96 parallelizable matmul operations per token.

The library already provides `fun_thread_pool` (simple fire-and-forget work queue with `{data, work_fn}`), `fun_sync` (mutex + condition variable), and `fun_async` (poll-based async with await). No existing module provides a reusable task-graph abstraction with dependencies or barriers.

The design follows the library's conventions: caller-allocated memory via `fun_memory_allocate`, direct function-pointer dispatch, no dynamic allocation in the hot path, and platform-abstracted threading via `arch/`.

## Goals / Non-Goals

**Goals:**

- A library-level compute-graph executor that decouples work description (DAG of tasks) from execution (1..N workers).
- Caller-allocated memory: graph and task metadata live in caller-provided buffers.
- Graph built once, executed repeatedly: the gpt-demo builds the graph at model load and submits it per token.
- Bind mechanism: per-task dynamic state updated at submit time via caller-provided bind functions.
- Use the existing `fun_thread_pool` internally for worker management; expose no new threading primitives.
- Single-threaded fallback (n_threads=0 or 1) with zero queue overhead.
- Backend extensibility: the `fn` function pointer is the abstraction boundary. CPU AVX2, GPU CUDA, or hybrid backends plug in by changing the function pointers on tasks — graph shape, submit, and wait are unchanged.

**Non-Goals:**

- A general-purpose task scheduler or job system. The graph is designed for dataflow-style compute (fixed DAG, many repeated executions).
- Heterogeneous device scheduling (automatic CPU/GPU placement). The caller assigns tasks to backends by choosing function pointers.
- Dynamic task creation during execution. The graph is immutable after construction.
- Serialization of the graph for remote execution. The function-pointer design enables it, but no wire format is defined.
- Replacing `fun_thread_pool`. The compute graph wraps it.

## Decisions

### 1. Task model: fn + ctx + bind + destroy

```c
typedef void (*FunComputeFn)(void *ctx);
typedef void (*FunComputeBindFn)(void *task_ctx, void *submit_ctx);
typedef void (*FunComputeCtxDestroyFn)(void *ctx);
```

Every task carries four function pointers:

- `fn(ctx)`: executes the computation. Writes results directly into buffers pointed to by the context struct.
- `bind(task_ctx, submit_ctx)`: called once per `submit`, before any `fn` runs. Copies dynamic fields (token position, buffer pointers, etc.) from the submission context into the task context. NULL if the task has only static data.
- `destroy(ctx)`: frees the context struct when the graph is destroyed. NULL if the context is static or caller-managed.

**Rationale:** The context struct holds both static fields (weight pointers, dimensions — set at graph build) and dynamic fields (input/output buffer pointers — populated by bind). The function pointer `fn` is the universal adapter for any backend: the same graph shape works for CPU AVX2 functions, CUDA kernel launchers, or even a hybrid mix.

**Alternatives considered:**

- Enum-based dispatch (OP_MATVEC, OP_ROTARY, ...): requires the executor to know all op types. Function pointers add ops without modifying the executor.
- Generic `void *data, size_t data_size` work items (like `fun_thread_pool`): forces the caller to pack/unpack arguments, losing type safety in context structs.

### 2. Caller-allocated memory

```c
size_t fun_compute_graph_memory_required(int max_tasks, int max_edges, int n_threads);
FunComputeGraph *fun_compute_graph_init(void *memory, size_t memory_size,
                                         int max_tasks, int max_edges, int n_threads);
```

The caller queries the required size with `memory_required`, allocates via `fun_memory_allocate`, and passes the buffer to `init`. The returned pointer is the same memory, typed.

**Rationale:** Aligns with the library's caller-allocation requirement. No internal allocations at graph-build time. The underlying `fun_thread_pool` is created inside the provided memory block. The caller frees the block after `fun_compute_graph_destroy`.

**Alternatives considered:**

- Library-internal allocation with `fun_compute_graph_create(&out_graph)`: contradicts the library's caller-allocation principle and adds error paths to the hot path.
- Separate allocations for graph, edges, queue, thread_pool: complicates the caller's teardown. A single block is simpler.

### 3. Explicit dependency edges

```c
void fun_compute_graph_add_task(FunComputeGraph *graph, FunComputeTask *task,
                                 FunComputeFn fn, void *ctx,
                                 FunComputeBindFn bind, FunComputeCtxDestroyFn destroy);
void fun_compute_task_depends_on(FunComputeGraph *graph,
                                  FunComputeTask *task, FunComputeTask *dep);
```

Tasks with zero dependencies (roots) are enqueued immediately on submit. A task with dependencies waits until all its `dep` tasks have completed. The completion process: `fn(ctx)` returns → for each dependent, decrement its remaining-dep counter → if counter reaches zero, enqueue the dependent.

**Rationale:** DAGs express the maximum parallelism available. In the gpt-demo, Q/K/V matvecs are independent (3 roots), 4 experts are independent after the router (4 parallel), KV store depends on RoPE_K and V (2 edges). Phase barriers would serialize at each boundary, losing overlap between, e.g., the router (fast) and experts (slow).

**Alternatives considered:**

- Phase barriers (`submit_batch` → wait → next batch): simpler but loses parallelism. With phase barriers, the router must complete before any expert starts, even though experts don't depend on the router output until the accumulation step.
- Implicit parent parameter in `add_task`: obscures multi-parent edges (KV store depends on RoPE_K + V), forcing those exceptions into a separate API call regardless.

### 4. Bind at submit time

```c
void fun_compute_graph_submit(FunComputeGraph *graph, void *submit_ctx);
```

On each `submit`, the executor iterates all tasks, calls `bind(task->ctx, submit_ctx)` for each non-NULL bind, resets dependency counters, and enqueues root tasks. This runs on the calling thread before any worker starts.

**Rationale:** The bind is the only place where per-execution state enters the graph. By running it synchronously before workers start, there is no race between bind and execution. The submit_ctx is a simple struct (e.g., `{int token_id, int pos}`) that the caller fills per token.

**Alternatives considered:**

- Bind per-task lazily (when the task is dequeued): requires synchronizing bind with the caller's next submit (the caller may overwrite submit_ctx for the next token). Synchronous bind-at-submit avoids this.
- Separate "update context" API before submit: two calls instead of one, no benefit.

### 5. Graph lifetime and re-execution

The graph is built once, submitted many times. Between `wait()` and the next `submit()`, the caller reads results from shared buffers and prepares the next `submit_ctx`. The graph's task contexts preserve their static fields across executions; only the bind-updated fields change.

**Rationale:** The gpt-demo's forward pass shape is fixed per model. Building the graph at `model_load` and reusing it per token avoids per-token allocation overhead. The task metadata (~300 tasks × ~80 bytes ≈ 24KB) is a fraction of the model's memory footprint.

### 6. Single-threaded fallback

When `n_threads = 0` or `1`, the executor runs tasks inline on the calling thread during `wait()`:

```algorithm
wait():
  while tasks pending:
    pick next ready task
    fn(ctx)
    decrement dependents
```

No thread pool is created. No locks acquired. This is the zero-overhead path for single-threaded use and serves as the correctness baseline against which multi-threaded output is verified.

### 7. Integration with existing thread_pool

The compute graph wraps `fun_thread_pool` for worker management. The graph's internal ready-queue pushes tasks as `WorkItem{data=task, data_size=sizeof(task), work_fn=_compute_worker}`. The worker function unwraps the task, calls `task->fn(task->ctx)`, then handles dependency resolution (decrement dependents, enqueue newly-ready tasks).

**Rationale:** Reuses the existing thread pool's lifecycle (create, submit, destroy), distribution (round-robin), and synchronization. The compute graph adds only the DAG layer on top.

## Risks / Trade-offs

- **[DAG building is manual]** → The caller must wire dependencies correctly. A missing edge causes a data race (task B reads a buffer before task A writes it). Mitigation: the single-threaded fallback is always available for correctness verification; multi-threaded results must match it token-for-token.
- **[Task granularity]** → Too-fine tasks (e.g., separate tasks for RMS norm, a 3μs operation) incur queue overhead disproportionate to their work. Mitigation: the design doesn't force any granularity; the graph builder chooses. The gpt-demo coarsens tiny operations (e.g., RMS norm fused into the following matvec).
- **[Thread count vs. graph shape]** → The gpt-demo has 4 experts, not 16 or 128. With 4 workers on 4 experts, the parallelism is perfect. Adding more workers provides no further gain on the expert path. The attention chain remains sequential regardless of thread count. Mitigation: documented. The graph itself can express finer parallelism (12 matvecs per layer if gate/up/down are separate tasks), enabling more workers to contribute when the hardware supports it.
- **[Bind runs on the calling thread]** → For very large graphs, iterating all tasks to call bind adds latency before workers start. Mitigation: bind functions are trivial (pointer assignments, a few integer copies). For the gpt-demo's ~300 tasks, bind overhead is <1μs.
