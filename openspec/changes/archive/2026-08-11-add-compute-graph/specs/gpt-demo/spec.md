# Specifications

## ADDED Requirements

### Requirement: Forward pass uses compute graph

The gpt-demo SHALL build a `FunComputeGraph` at model load time representing the full forward pass, and SHALL execute it once per token via `fun_compute_graph_submit` and `fun_compute_graph_wait`. The graph SHALL encode all layer-by-layer operations (attention RMS norm, Q/K/V matvecs, RoPE, KV cache store, attention scores, output projection, residual add, FFN norm, router, expert MXFP4 projections, SiLU gating, weighted accumulation) as tasks with explicit dependency edges.

#### Scenario: Graph built once at model load

- **WHEN** `model_load` completes
- **THEN** the compute graph is fully constructed with all tasks and dependencies for 24 layers

#### Scenario: Graph executed per token

- **WHEN** `model_forward` is called for a token
- **THEN** it calls `fun_compute_graph_submit` and `fun_compute_graph_wait` exactly once

#### Scenario: Single-threaded output matches baseline

- **WHEN** the graph is executed with `n_threads = 0`
- **THEN** the greedy argmax token sequence SHALL match the pre-refactor single-threaded baseline for the same prompt and model

#### Scenario: Multi-threaded output matches single-threaded

- **WHEN** the graph is executed with `n_threads = 4`
- **THEN** the greedy argmax token sequence SHALL match the `n_threads = 0` output for the same prompt and model

### Requirement: Expert parallelism via task graph

The gpt-demo SHALL add the four per-layer expert tasks as independent nodes in the graph, all depending on the post-attention normalization task. The expert accumulation task SHALL depend on all four expert tasks and the router task. This SHALL allow worker threads to execute up to four experts concurrently.

#### Scenario: Experts are parallel in the graph

- **WHEN** the graph is inspected after `model_load`
- **THEN** for each layer, four expert tasks exist with no dependency edges between them, and all four depend on the same FFN normalization task

#### Scenario: Accumulate waits for all experts

- **WHEN** the graph is inspected after `model_load`
- **THEN** the expert accumulation task for each layer depends on all four expert tasks and the router task
