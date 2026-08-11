# Proposal

## Why

The gpt-demo shows 60% CPU utilization with 4 threads — well below the expected ~83%. The coordination overhead of the compute graph's fine-grained DAG (per-task mutex/condvar, phase gaps) is suspected but unmeasured. Without per-phase timing instrumentation, optimization decisions are guesswork. The library lacks any cross-platform high-resolution timing primitive and any named-counter profiling facility.

## What Changes

- New `fundamental/timing` module: provides `fun_timing_now_ns()` — a cross-platform monotonic nanosecond clock. Windows uses `QueryPerformanceCounter`, Linux uses `clock_gettime(CLOCK_MONOTONIC)`. No init required.
- New `fundamental/trace` module — an OpenTelemetry-aligned tracer with Span registration, begin/end scoped timing, typed attributes (i64, f64, string), discrete events, and formatted console export. Thread-safe via `_Thread_local` start timestamps and `__atomic` accumulation. Caller-allocated memory. Before `fun_trace_init`, all calls are transparent no-ops; after `fun_trace_destroy`, they become no-ops again.
- **Instrumented gpt-demo**: model.c exec functions wrap their work with `fun_trace_span_begin`/`end`. Expert spans carry `layer` and `expert_idx` attributes. `model_forward` wraps submit/wait with an `inference` span. A final report prints per-phase breakdowns and computes coordination overhead. Build scripts gain `src/trace/trace.c` and `arch/timing/` sources.
- Demo-level `timing.h` and `arch/timing/` files removed — replaced by library-level `fun_timing`.

## Capabilities

### New Capabilities

- `timing`: Cross-platform monotonic nanosecond clock. Single function: `fun_timing_now_ns()`. Arch-specific implementations in `arch/timing/{windows-amd64,linux-amd64}/timing.c`.
- `trace`: OpenTelemetry-aligned tracing with `FunTracer`, `FunSpanId`, span registration, begin/end scoped timing, typed attributes (i64/f64/str), discrete events, and formatted console report. Caller-allocated memory. Global singleton tracer. No-ops when uninitialized.

### Modified Capabilities

- `gpt-demo`: Exec functions instrumented with span begin/end pairs. Per-token submit/wait timed as `inference` span. Expert spans carry layer/expert_idx attributes. Tracer initialized at startup, report printed after generation. Demo timing replaced by library `fun_timing`. Demo-local timing files removed.

## Impact

- **New modules**: `include/fundamental/timing/timing.h`, `arch/timing/{windows-amd64,linux-amd64}/timing.c`, `include/fundamental/trace/trace.h`, `src/trace/trace.c`, `tests/trace/`
- **Modified demo**: `demos/gpt-demo/model.c` (span begin/end in exec functions), `demos/gpt-demo/main.c` (tracer init and report, `fun_timing_now_ns`), `demos/gpt-demo/build-windows-amd64.bat` and `build-linux-amd64.sh` (new source files)
- **Removed**: `demos/gpt-demo/timing.h`, `demos/gpt-demo/arch/timing/` — replaced by library-level `fun_timing`
- No breaking changes to existing public APIs.
