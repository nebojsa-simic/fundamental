# compute Specification

## Purpose
TBD - created by archiving change add-compute-graph. Update Purpose after archive.
## Requirements
### Requirement: Caller queries memory required for graph

The system SHALL provide `fun_compute_graph_memory_required(max_tasks, max_edges, n_threads)` that returns the number of bytes needed to initialize a compute graph with the given capacities. The returned size SHALL include space for task metadata, dependency edge storage, a ready-queue, and an internal `fun_thread_pool` when `n_threads > 0`.

#### Scenario: Returns positive size for valid parameters

- **WHEN** `fun_compute_graph_memory_required(100, 200, 4)` is called
- **THEN** the return value SHALL be greater than 0

#### Scenario: Zero threads returns valid size

- **WHEN** `fun_compute_graph_memory_required(100, 200, 0)` is called
- **THEN** the return value SHALL be greater than 0 and SHALL be smaller than the same call with `n_threads = 4`

#### Scenario: Increasing capacities increases required memory

- **WHEN** `fun_compute_graph_memory_required(A, B, C)` and `fun_compute_graph_memory_required(A*2, B, C)` are compared
- **THEN** the larger max_tasks SHALL require at least as much memory

### Requirement: Graph is initialized in caller-provided memory

The system SHALL provide `fun_compute_graph_init(memory, memory_size, max_tasks, max_edges, n_threads)` that initializes a compute graph in the caller-provided buffer. The function SHALL return a `FunComputeGraph *` pointer into the buffer. The memory SHALL be at least the size returned by `fun_compute_graph_memory_required` for the same parameters. When `n_threads > 0`, an internal `fun_thread_pool` SHALL be created inside the buffer. When `n_threads` is 0, no thread pool SHALL be created and execution SHALL run inline on the calling thread during `wait`.

#### Scenario: Init returns pointer into caller's memory

- **WHEN** `fun_compute_graph_init(buf, size, 100, 200, 0)` is called
- **THEN** the returned pointer SHALL be within `[buf, buf + size)`

#### Scenario: Init with insufficient memory is undefined

- **WHEN** `fun_compute_graph_init` is called with `memory_size` smaller than the value returned by `fun_compute_graph_memory_required` for the same parameters
- **THEN** behavior is undefined (caller contract)

### Requirement: Tasks are added with function pointers and context

The system SHALL provide `fun_compute_graph_add_task(graph, task, fn, ctx, bind, destroy)` that registers a task in the graph. The task struct SHALL be caller-allocated. `fn` is the execution function (`void (*)(void *)`) invoked when all dependencies are satisfied. `ctx` is the opaque context passed to `fn`, `bind`, and `destroy`. `bind` (`void (*)(void *, void *)`) SHALL be called once per `submit` with the task context and the submission context before any `fn` executes. `destroy` (`void (*)(void *)`) SHALL be called once per `graph_destroy` on the context. All three function pointers MAY be NULL.

#### Scenario: Task added with valid parameters

- **WHEN** `fun_compute_graph_add_task(graph, &task, my_fn, &my_ctx, my_bind, my_destroy)` is called with all non-NULL function pointers
- **THEN** the task is registered in the graph and SHALL be executed on the next submit

#### Scenario: Task added with NULL bind and destroy

- **WHEN** bind and destroy are NULL
- **THEN** the task SHALL still execute normally; no bind is called on submit, no destroy is called on graph_destroy for that task

### Requirement: Dependencies are expressed as explicit edges

The system SHALL provide `fun_compute_task_depends_on(graph, task, dep)` that declares `task` SHALL NOT execute until `dep` has completed. A task with zero inbound edges is a root and SHALL be enqueued immediately on `submit`. A task SHALL NOT execute until all its inbound dependencies have completed. Cycles SHALL NOT be detected; adding a cycle is undefined behavior.

#### Scenario: Task waits for single dependency

- **WHEN** task B depends on task A, and submit is called
- **THEN** task B SHALL NOT execute before task A completes

#### Scenario: Task waits for multiple dependencies

- **WHEN** task C depends on both A and B, and submit is called
- **THEN** task C SHALL NOT execute before both A and B have completed

#### Scenario: Root task executes immediately

- **WHEN** a task has no dependencies and submit is called
- **THEN** the task SHALL be enqueued for execution immediately

### Requirement: Submit binds dynamic state and enqueues root tasks

The system SHALL provide `fun_compute_graph_submit(graph, submit_ctx)` that prepares the graph for execution. The function SHALL iterate all tasks, call `bind(task_ctx, submit_ctx)` for each non-NULL bind, reset dependency counters to their initial values, and enqueue all root tasks (those with zero dependencies) for execution. Submit SHALL be non-blocking: it returns before any `fn` executes. Submit SHALL be thread-safe only when called from a single thread between `wait` calls.

#### Scenario: Bind is called for every task with a bind function

- **WHEN** a graph with three tasks (all with non-NULL bind) is submitted
- **THEN** all three bind functions SHALL be invoked exactly once each with the submit_ctx, before any fn executes

#### Scenario: Bind not called for tasks without bind function

- **WHEN** a task has NULL bind and submit is called
- **THEN** no bind function is invoked for that task

#### Scenario: Submit returns before execution

- **WHEN** `fun_compute_graph_submit` is called
- **THEN** the function SHALL return to the caller before any task's `fn` has been invoked

### Requirement: Wait blocks until all tasks complete

The system SHALL provide `fun_compute_graph_wait(graph)` that blocks the calling thread until every task in the graph has completed execution. When `n_threads` is 0, the calling thread SHALL execute all tasks inline during `wait`. When `n_threads > 0`, worker threads execute tasks while the calling thread blocks on a condition variable.

#### Scenario: Wait returns after all tasks complete

- **WHEN** `fun_compute_graph_wait` is called after submit
- **THEN** the function SHALL return only after every task in the graph has had its `fn` invoked and returned

#### Scenario: Multiple submit/wait cycles on same graph

- **WHEN** submit → wait → submit → wait is called twice on the same graph with different submit_ctx values
- **THEN** both executions SHALL complete successfully and results from the second execution SHALL reflect the second submit_ctx

#### Scenario: Single-threaded execution on calling thread

- **WHEN** the graph was initialized with `n_threads = 0` and wait is called
- **THEN** the calling thread SHALL execute all tasks inline; no worker threads SHALL be spawned

### Requirement: Graph is destroyed, freeing thread pool and calling destructors

The system SHALL provide `fun_compute_graph_destroy(graph)` that tears down the graph. The function SHALL call `destroy(ctx)` for every task with a non-NULL destroy function. If `n_threads > 0`, the internal `fun_thread_pool` SHALL be destroyed, blocking until in-progress tasks complete. The function SHALL NOT free the memory buffer passed to `init` — the caller owns it. After destroy, the graph pointer SHALL NOT be used.

#### Scenario: Destroy calls all task destructors

- **WHEN** `fun_compute_graph_destroy` is called on a graph with three tasks, all with non-NULL destroy
- **THEN** all three destroy functions SHALL be invoked exactly once each

#### Scenario: Destroy waits for in-progress tasks

- **WHEN** `fun_compute_graph_destroy` is called while worker threads are executing tasks
- **THEN** the function SHALL block until all in-progress `fn` calls have returned before returning

#### Scenario: Caller frees memory after destroy

- **WHEN** `fun_compute_graph_destroy` returns and the caller calls `fun_memory_free` on the original buffer
- **THEN** no use-after-free SHALL occur (library holds no references into the buffer after destroy)

