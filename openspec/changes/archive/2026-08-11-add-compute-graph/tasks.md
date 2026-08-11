# Tasks

## 1. Compute module: header

- [x] 1.1 Create `include/fundamental/compute/compute.h` with types (`FunComputeFn`, `FunComputeBindFn`, `FunComputeCtxDestroyFn`, `FunComputeGraph`, `FunComputeTask`), sizing function (`fun_compute_graph_memory_required`), init, add_task, depends_on, submit, wait, destroy
- [x] 1.2 Define internal struct layouts in `src/compute/compute_graph.c` (task metadata: fn/bind/destroy pointers, ctx pointer, dependency counters, adjacency lists, ready-queue)

## 2. Compute module: graph construction

- [x] 2.1 Implement `fun_compute_graph_memory_required` — sum sizes for task table, edge adjacency arrays, ready-queue, optional thread_pool
- [x] 2.2 Implement `fun_compute_graph_init` — carve the memory buffer into internal structures, initialize dependency counters, create thread_pool if `n_threads > 0`
- [x] 2.3 Implement `fun_compute_graph_add_task` — store fn/ctx/bind/destroy in the task table
- [x] 2.4 Implement `fun_compute_task_depends_on` — record edge, increment dependency counter on target task

## 3. Compute module: execution

- [x] 3.1 Implement `fun_compute_graph_submit` — iterate tasks calling bind, reset dependency counters to stored initial values, enqueue root tasks (zero deps)
- [x] 3.2 Implement single-threaded `fun_compute_graph_wait` (n_threads=0 path) — dequeue tasks, call `fn(ctx)`, decrement dependent counters, enqueue newly-ready tasks, block until all done
- [x] 3.3 Implement multi-threaded `fun_compute_graph_wait` (n_threads>0 path) — wrap `fun_thread_pool_submit` with worker function that calls `fn(ctx)` then resolves dependents, block on condition variable until all tasks done
- [x] 3.4 Implement `fun_compute_graph_destroy` — call all non-NULL destroy functions, destroy thread_pool if present

## 4. Compute module: internal mechanics

- [x] 4.1 Implement ready-queue with mutex-protected enqueue/dequeue for multi-threaded path
- [x] 4.2 Implement dependency resolution in worker: after `fn` returns, iterate task's dependents, atomically decrement counter, enqueue when counter reaches zero
- [x] 4.3 Track pending task count, signal condition variable when it reaches zero (unblocking `wait`)

## 5. Compute module: build and test

- [x] 5.1 Create `tests/compute/` directory with `test.c`, `build-windows-amd64.bat`, `build-linux-amd64.sh`
- [x] 5.2 Test: single task with no dependencies executes and writes result to shared buffer
- [x] 5.3 Test: chain A→B→C executes in order and C sees B's output
- [x] 5.4 Test: parallel roots (A, B both depend on nothing; C depends on both) — both A and B execute before C
- [x] 5.5 Test: multiple submit/wait cycles on same graph with different submit_ctx values produce correct results
- [x] 5.6 Test: graph_destroy calls all task destructors; caller frees memory after destroy without crash
- [x] 5.7 Test: single-threaded (n_threads=0) and multi-threaded (n_threads=2,4) produce identical results on a DAG with parallel branches
- [x] 5.8 Test: bind function is called exactly once per task per submit, before any fn executes

## 6. gpt-demo: graph construction in model_load

- [x] 6.1 Define context structs for each op type (CtxRmsNorm, CtxMatvecF32, CtxRotary, CtxKvStore, CtxAttention, CtxRouter, CtxExpert, CtxExpertAccumulate, CtxOutputProj) in `model.c`
- [x] 6.2 Define execution functions for each op type calling existing `fun_math_*` primitives
- [x] 6.3 Define bind functions where needed (embedding dequant, rope precompute, KV cache pos, buffer pointer updates)
- [x] 6.4 In `model_load`, query `fun_compute_graph_memory_required`, allocate buffer, init graph, add all tasks with dependencies for 24 layers
- [x] 6.5 Wire expert parallelism: four expert tasks per layer, all depending on FFN norm, all feeding into accumulate task (which also depends on router)

## 7. gpt-demo: per-token execution

- [x] 7.1 Replace `model_forward` body with: build `SubmitCtx {token_id, pos}`, call `submit`, call `wait`, sample logits
- [x] 7.2 Pre-compute RoPE angles as graph tasks (t_embed, t_rope_pre) rather than in the main loop
- [x] 7.3 Remove per-layer `dequant_layer` lazy-loading — layers are fully loaded at model_load (graph construction requires weight pointers upfront)
- [x] 7.4 Remove scratch buffer allocations from forward pass (graph tasks use pre-allocated reuse buffers)

## 8. gpt-demo: build scripts

- [x] 8.1 Update `demos/gpt-demo/build-windows-amd64.bat` — add `../../src/compute/compute_graph.c`, `../../src/thread_pool/thread_pool.c`, and relevant arch files (thread_pool platform sources)
- [x] 8.2 Update `demos/gpt-demo/build-linux-amd64.sh` — same source additions for Linux

## 9. gpt-demo: documentation

- [x] 9.1 Create `demos/gpt-demo/docs/architecture.md` with: inference pipeline overview diagram, forward pass per-token computation breakdown with FLOP/byte estimates, parallelism shape (expert independence, attention chain), compute graph task layout, backend extensibility (CPU AVX2 → multi-threaded → GPU), single-threaded vs multi-threaded speed table

## 10. Validation

- [x] 10.1 Build gpt-demo with single-threaded graph (n_threads=0), verify greedy output matches current baseline for a known prompt
- [x] 10.2 Build and run gpt-demo with n_threads=4, verify greedy output matches single-threaded
- [x] 10.3 Measure tok/s with 1, 2, and 4 threads, verify speedup scales with thread count on the expert path
- [x] 10.4 Run `../../demos/validate-demo.bat` to verify no regressions
- [x] 10.5 Run full test suite (`run-tests-windows-amd64.bat`) to verify no regressions in existing modules
