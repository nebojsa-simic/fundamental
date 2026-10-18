# Design

## Context

The gpt-demo refactored to use the compute graph runs at 2.22 tok/s multi-threaded (4 workers) on the target machine, with identical output to the single-threaded baseline. Task Manager shows ~60% CPU utilization — not the ~83% expected from the compute shape (22% sequential attention, 78% parallel experts). The gap is suspected to be coordination overhead from the fine-grained DAG (~485 tasks per token, mutex/condvar on every task completion, idle gaps between phases).

The library lacks any timing or profiling primitive. The gpt-demo has a demo-local `arch/timing/` with `demo_time_now()` — a non-reusable pattern. A library-level timing module (`fun_timing`) enables a tracing module (`fun_trace`) modeled after OpenTelemetry concepts (Tracer, Span, Attributes, Events), which enables precise measurement of where time is spent.

## Goals / Non-Goals

**Goals:**

- Library-level cross-platform monotonic nanosecond clock (`fun_timing`).
- An OpenTelemetry-aligned tracing module (`fun_trace`) with Span registration, begin/end scoped timing, typed attributes, discrete events, and formatted console export.
- Caller-allocated memory for the tracer registry.
- Minimal overhead: `fun_trace_span_begin`/`end` are a NULL check + `fun_timing_now_ns()` + TLS store on begin, atomic fetch-add on end. ~20-50ns on modern x86.
- Instrument the gpt-demo's exec functions to measure per-phase time, and compute coordination overhead.
- Demo-only instrumentation — no changes to the compute graph or math library code.

**Non-Goals:**

- A full OTEL SDK (no context propagation, no W3C trace context, no distributed tracing, no OTLP exporter).
- Sampling, histograms, percentile calculations.
- JSON or structured output for the initial MVP — text table via callback.
- `fun_timing` calibration, frequency conversion, or sleep/delay functions.

## Decisions

### 1. OpenTelemetry-aligned API

```ascii
  OTEL                        fun_trace
  ────────────────────────    ─────────────────────
  TracerProvider / Tracer     FunTracer (global singleton)
  Span                        Span (aggregated by name, many instances)
  StartSpan / EndSpan         fun_trace_span_begin(id) / fun_trace_span_end(id)
  SetAttribute                fun_trace_span_attribute_*(id, key, value)
  AddEvent                    fun_trace_span_add_event(id, event_name)
  SpanProcessor / Exporter    fun_trace_report(output_fn)
```

```c
typedef struct FunTracer FunTracer;   // opaque
typedef uint32_t        FunSpanId;

// ── Lifecycle ──
size_t fun_trace_memory_required(int max_spans);
void   fun_trace_init(void *memory, size_t size, int max_spans);
void   fun_trace_destroy(void);

// ── Span registration ──
FunSpanId fun_trace_span_register(const char *name);

// ── Span lifecycle ──
void fun_trace_span_begin(FunSpanId id);
void fun_trace_span_end(FunSpanId id);

// ── Attributes (set after begin, before end, or accumulated) ──
void fun_trace_span_attribute_i64(FunSpanId id, const char *key, int64_t v);
void fun_trace_span_attribute_f64(FunSpanId id, const char *key, double v);
void fun_trace_span_attribute_str(FunSpanId id, const char *key, const char *v);

// ── Events ──
void fun_trace_span_add_event(FunSpanId id, const char *event_name);

// ── Export ──
typedef void (*FunTraceReportFn)(const char *line);
void fun_trace_report(FunTraceReportFn out);

// ── Counters (non-span accumulators) ──
void fun_trace_counter_add(FunSpanId id, int64_t delta);
```

**Rationale:** Using OTEL terminology makes intent obvious to anyone familiar with observability standards. The Span concept maps naturally: register a name once, begin/end many times, accumulate atomically. Attributes enrich a span with per-occurrence metadata (e.g., `layer=3`, `expert_idx=2`). Events count discrete occurrences (e.g., cache misses).

### 2. Attribute model

Attributes are per-Span, per-key accumulators. They use a fixed-size storage of key-value pairs associated with each span ID.

```c
// Internal per-span storage:
typedef struct {
    const char *name;
    uint64_t total_ns;
    uint64_t call_count;
    int64_t  counter;
    struct {
        const char *key;
        int64_t     i64_val;
        double      f64_val;
        const char *str_val;
        int         type;   // 0=unset, 1=i64, 2=f64, 3=str
    } attrs[8];              // fixed max attributes per span
    int nattrs;
} SpanSlot;
```

**Rationale:** OTEL attributes are key-value pairs set on a span. For an MVP, a fixed-size (8) attribute array per span avoids dynamic allocation. Common use: `layer={N}`, `expert_idx={e}`, `thread={T}`. When the same key is set multiple times per span occurrence, the last write wins (not accumulated).

**Alternatives considered:** No attributes (skip until needed): but the gpt-demo needs per-layer/per-expert breakdown to understand expert parallelism utilization. Attributes enable that without registering separate span names for every layer×expert combination.

### 3. Global registry, TLS start timestamp

```c
static FunTracerInternal *g_tracer = NULL;
_Thread_local uint64_t _trace_t0 = 0;
```

Before init, all calls check `g_tracer` and return immediately. After init, `begin` stores the timestamp in TLS. `end` reads TLS and atomically accumulates into the span slot.

### 4. Timing module: one function, arch directories

```ascii
include/fundamental/timing/timing.h
arch/timing/windows-amd64/timing.c    → QueryPerformanceCounter
arch/timing/linux-amd64/timing.c      → clock_gettime(CLOCK_MONOTONIC)
```

```c
uint64_t fun_timing_now_ns(void);
```

The existing demo-local `arch/timing/` pattern graduates to the library. This replaces `demo_time_now()` and its demo-local arch files.

### 5. gpt-demo: phase grouping with span names

| Span name | Tasks covered |
| ----------- | -------------- |
| `inference` | entire submit→wait span in model_forward |
| `attention` | RMS norm, Q/K/V matvecs, RoPE, KV store, attn scores, output proj, residual add, copy |
| `expert` | gate+up MXFP4, SiLU gating, down MXFP4 per expert |
| `router` | router logits + top-k selection + softmax weights |
| `output` | output RMS norm + Q8 vocab projection |
| `embed` | token embedding dequant |
| `rope` | RoPE angle precomputation |

Attributes added to `expert` spans: `layer=<l>`, `expert_idx=<e>`. This enables per-layer expert time analysis without 96 separate span registrations.

### 6. Report output format

```ascii
Span                 Total (ms)   Count   Avg (μs)      %
─────────────────────────────────────────────────────────
inference              28700         64    448438     100%
  expert               18400       3840      4792      64%
    layer=0              800        160      5000
    layer=1              780        160      4875
  attention             3800        768      4948      13%
  router                 310        768       404       1%
  output                 160         64      2500      <1%
  embed                    2         64        31      <1%
─────────────────────────────────────────────────────────
  overhead              6028                           21%
```

Attributes printed as indented sub-rows when they vary across occurrences.

## Risks / Trade-offs

- **[Attribute storage is per-span]** → Adding `layer=N` attribute to an `expert` span means all 4 experts in all 24 layers share the same 8-slot attribute storage. The last-set values win for each key. Detailed per-layer attribution requires post-processing or more span names. Mitigation: the MVP uses attributes for at-a-glance per-layer min/max, not per-layer total.
- **[Single nesting level]** → Only one active span per thread. Mitigation: gpt-demo's phases are flat. The `inference` span is computed separately (not nested within exec functions).
