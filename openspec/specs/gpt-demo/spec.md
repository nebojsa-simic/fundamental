# gpt-demo Specification

## Purpose
TBD - created by archiving change gpt-demo. Update Purpose after archive.

## Requirements

### Requirement: GPT-OSS-20B demo application
The system SHALL provide a CLI demo at `demos/gpt-demo/` that loads a GPT-OSS-20B GGUF model and performs single-request inference.

#### Scenario: Command-line prompt
- **WHEN** `demo.exe "Hello how are you?"` is executed
- **THEN** the demo prints a model-generated response to stdout

#### Scenario: Timing output
- **WHEN** the inference completes
- **THEN** the demo prints evaluation time in seconds and tokens per second

#### Scenario: Missing model file
- **WHEN** the GGUF model file is not found at the expected path
- **THEN** the demo prints an error message and exits with non-zero code

### Requirement: Fused MXFP4 expert inference
The gpt-demo SHALL compute each selected MoE expert's gate/up/down projections
with `fun_math_matrix_vector_mxfp4_f32` directly over the memory-mapped GGUF
tensor data, and SHALL NOT dequantize expert weights into f32 heap buffers,
materialize per-expert cache files, or look up tensor names in the forward
pass.

#### Scenario: Per-layer tensor bases precomputed
- **WHEN** a layer is first loaded
- **THEN** the demo stores the GGUF offsets of the gate/up/down MXFP4 weight
  tensors and their f32 bias tensors, plus the byte stride per expert, and the
  forward pass indexes into them without name lookups

#### Scenario: Expert projections use the fused primitive
- **WHEN** expert `e` in layer `l` is selected
- **THEN** the gate and up projections are computed with
  `fun_math_matrix_vector_mxfp4_f32` against the hidden state and the down
  projection against the intermediate activation, each with the expert's bias
  vector

#### Scenario: No persistent cache
- **WHEN** generation runs
- **THEN** no expert weight files are read or written and no cache state is
  consulted; each expert's weights are read from the GGUF mapping on use

#### Scenario: Greedy output preserved
- **WHEN** generation runs after the refactor with the same prompt and model
- **THEN** the greedy argmax token sequence is unchanged from the pre-refactor
  baseline (no quantization drift)

### Requirement: Cross-platform demo build
The gpt-demo SHALL build on both Windows and Linux from platform-specific
build scripts that select platform-specific arch files, and SHALL contain no
operating-system-specific code in its shared source files.

#### Scenario: Windows build
- **WHEN** `build-windows-amd64.bat` is executed
- **THEN** it compiles the demo with the windows-amd64 timing arch file into
  `demo.exe`

#### Scenario: Linux build
- **WHEN** `build-linux-amd64.sh` is executed
- **THEN** it compiles the demo with the linux-amd64 timing arch file into a
  Linux executable using `gcc`

#### Scenario: Portable timing
- **WHEN** the demo measures elapsed time
- **THEN** it uses the platform's monotonic clock through a demo-local timing
  abstraction, not a direct OS API call in shared code

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

### Requirement: Exec functions are instrumented with spans

The gpt-demo SHALL call `fun_trace_span_begin` and `fun_trace_span_end` in each exec function in `model.c`. The span names SHALL be registered at model load time. Each exec function SHALL wrap its compute work in a begin/end pair.

#### Scenario: Expert exec function timed

- **WHEN** `_exec_expert` is called
- **THEN** it SHALL call `fun_trace_span_begin(SPAN_EXPERT)` before computation and `fun_trace_span_end(SPAN_EXPERT)` after

#### Scenario: Expert span has layer and expert_idx attributes

- **WHEN** `_exec_expert` completes
- **THEN** it SHALL set `layer` and `expert_idx` attributes via `fun_trace_span_attribute_i64`

### Requirement: Model forward timed with inference span

The gpt-demo SHALL wrap the `submit`/`wait` call in `model_forward` with `fun_trace_span_begin(SPAN_INFERENCE)` and `fun_trace_span_end(SPAN_INFERENCE)`. The span time SHALL include bind, execution, and all coordination overhead.

#### Scenario: Inference span covers submit and wait

- **WHEN** `model_forward` is called for a token
- **THEN** the elapsed time between `span_begin(INFERENCE)` before `submit` and `span_end(INFERENCE)` after `wait` SHALL be recorded

### Requirement: Tracer initialized at startup

The gpt-demo SHALL call `fun_trace_memory_required`, `fun_memory_allocate`, and `fun_trace_init` during `main()` before model loading. Span names SHALL be registered during `model_load`.

#### Scenario: Tracer initialized before model load

- **WHEN** demo starts
- **THEN** `fun_trace_init` SHALL be called before `model_load`

### Requirement: Report printed after generation completes

The gpt-demo SHALL call `fun_trace_report` after generation completes (before model_free), outputting a per-phase timing breakdown to the console that includes span names, total times, call counts, and computed coordination overhead percentage.

#### Scenario: Report printed after generation

- **WHEN** generation loop exits (EOS token or max tokens reached)
- **THEN** `fun_trace_report` SHALL be called, printing a formatted table to stdout

### Requirement: Demo timing replaced by library timing

The gpt-demo SHALL use `fun_timing_now_ns()` instead of `demo_time_now()`. The demo-local `timing.h` and `arch/timing/` files SHALL be removed.

#### Scenario: Library timing used for elapsed time

- **WHEN** the demo measures elapsed time
- **THEN** it SHALL use `fun_timing_now_ns()` from `fundamental/timing/timing.h`
