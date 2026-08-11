# Proposal

## Why

The gpt-demo runs at 0.45 tok/s on a single CPU core. The dominant cost (~78% of compute) is MoE expert inference — four independent experts per layer, each doing three MXFP4 matrix-vector products — which is embarrassingly parallel but currently serial. Adding multi-threading to the demo directly would couple it to a specific parallelism strategy and block future backend swaps (more threads, GPU, CPU+GPU hybrid). A library-level compute graph primitive decouples the work description (what to compute, which buffers, what order) from the execution backend (how many threads, what device), enabling the demo and any future consumer to scale without rewriting the parallelism logic.

## What Changes

- New `fundamental/compute` module providing a task-graph executor:
  - **`FunComputeGraph`**: a directed acyclic graph of tasks, built once and executed repeatedly. Tasks carry a function pointer (`fn`), an opaque caller-allocated context (`ctx`), an optional bind function (`bind`) to refresh dynamic fields before each execution, and an optional destructor (`destroy`).
  - **`fun_compute_graph_memory_required`**: caller queries the memory needed for a graph of given task/edge/thread counts.
  - **`fun_compute_graph_init`**: initializes the graph in caller-provided memory.
  - **`fun_compute_graph_add_task`** / **`fun_compute_task_depends_on`**: build the DAG.
  - **`fun_compute_graph_submit`** / **`fun_compute_graph_wait`**: bind dynamic state and execute. Non-blocking submit, blocking wait.
  - **`fun_compute_graph_destroy`**: tears down the internal thread pool and calls task destructors. Caller frees the memory thereafter.
- **New `demos/gpt-demo/docs/architecture.md`**: architecture diagrams and explanation of the inference pipeline, forward pass shape, parallelism opportunities, and backend extensibility.
- **Refactored `demos/gpt-demo/model.c`**: uses the compute graph for expert parallelism. Per-layer attention and FFN tasks are defined at model load and executed per token via submit/wait. Single-threaded mode (1 worker) produces identical output to the current implementation.
- Existing `fun_thread_pool` is used internally by the executor. No changes to the thread_pool API.

## Capabilities

### New Capabilities
- `compute`: Task-graph executor. Caller builds a DAG of tasks (function pointer + context + bind + destructor), submits it with dynamic state, waits for completion. Supports 1 to N worker threads. Library-internal use of `fun_thread_pool`.

### Modified Capabilities
- `gpt-demo`: Forward pass restructured to use the compute graph. Expert parallelism via multiple worker threads. Output text unchanged from current single-threaded baseline.

## Impact

- **New module**: `include/fundamental/compute/compute.h`, `src/compute/compute_graph.c`, `arch/compute/` (platform thread primitives via existing `fun_thread_pool` and `fun_sync`), `tests/compute/` (functional and threading tests).
- **Modified demo**: `demos/gpt-demo/model.c`, `model.h` — restructured forward pass; `demos/gpt-demo/build-windows-amd64.bat`, `build-linux-amd64.sh` — link `src/compute/compute_graph.c` and `src/thread_pool/thread_pool.c`.
- **New doc**: `demos/gpt-demo/docs/architecture.md`.
- No breaking changes to existing public APIs.
