# Specifications

## ADDED Requirements

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
