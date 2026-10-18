# Tasks

## 1. fun_timing module

- [x] 1.1 Create `include/fundamental/timing/timing.h` with `uint64_t fun_timing_now_ns(void)`
- [x] 1.2 Create `arch/timing/windows-amd64/timing.c` — `QueryPerformanceCounter` + `QueryPerformanceFrequency` (cached on first call), convert to nanoseconds
- [x] 1.3 Create `arch/timing/linux-amd64/timing.c` — `clock_gettime(CLOCK_MONOTONIC, &ts)`, convert to nanoseconds

## 2. fun_trace module: header and implementation

- [x] 2.1 Create `include/fundamental/trace/trace.h` with `FunSpanId`, `FunTraceReportFn`, and all API functions (memory_required, init, destroy, span_register, span_begin, span_end, attribute_i64/f64/str, add_event, counter_add, report)
- [x] 2.2 Define internal structs in `src/trace/trace.c`: `SpanSlot` with name, total_ns, call_count, counter, attribute array (8 slots), event names. `FunTracerInternal` with span slots array, count, global pointer
- [x] 2.3 Implement `fun_trace_memory_required` — compute size of internal header + span array
- [x] 2.4 Implement `fun_trace_init` — carve memory, zero slots, set global pointer
- [x] 2.5 Implement `fun_trace_destroy` — null global pointer
- [x] 2.6 Implement `fun_trace_span_register` — O(n) lookup by name, return existing or assign new ID
- [x] 2.7 Implement `fun_trace_span_begin` — NULL check, store `fun_timing_now_ns()` in `_Thread_local _trace_t0`
- [x] 2.8 Implement `fun_trace_span_end` — NULL check, compute elapsed, atomic fetch-add to total_ns and call_count
- [x] 2.9 Implement `fun_trace_span_attribute_i64`/`fun_trace_span_attribute_f64`/`fun_trace_span_attribute_str` — store in per-span attribute array (8 slots, last-write-wins, silent drop when full)
- [x] 2.10 Implement `fun_trace_span_add_event` — atomic increment of event counter
- [x] 2.11 Implement `fun_trace_counter_add` — atomic fetch-add to span counter
- [x] 2.12 Implement `fun_trace_report` — format text table, call output_fn per line. Compute summary: first span as total, overhead = total - sum(others)

## 3. fun_trace: tests

- [x] 3.1 Create `tests/trace/` directory with `test.c`, `build-windows-amd64.bat`, `build-linux-amd64.sh`
- [x] 3.2 Test: uninitialized tracer — all calls are no-ops, report produces no output
- [x] 3.3 Test: span register returns same ID for same name
- [x] 3.4 Test: span begin/end accumulates correct elapsed time
- [x] 3.5 Test: concurrent span ends from multiple threads produce correct totals (no data races)
- [x] 3.6 Test: attributes — i64, f64, str stored correctly, last-write-wins for same key
- [x] 3.7 Test: report produces formatted text with expected columns
- [x] 3.8 Test: counter_add accumulates correctly
- [x] 3.9 Test: add_event increments event count
- [x] 3.10 Test: init → destroy → re-init works

## 4. gpt-demo: instrumentation

- [x] 4.1 Register span names in `model_load`: "inference", "attention", "expert", "router", "output", "embed", "rope"
- [x] 4.2 Wrap `_exec_expert` with span begin/end, set layer and expert_idx attributes
- [x] 4.3 Wrap remaining exec functions (`_exec_embed`, `_exec_rms_norm`, `_exec_matvec_f32`, `_exec_rotary`, `_exec_kvstore`, `_exec_attention`, `_exec_router`, `_exec_expert_accum`, `_exec_rope_pre`, `_exec_output_proj`) with span begin/end
- [x] 4.4 Wrap `model_forward` submit→wait with span begin/end for "inference"
- [x] 4.5 In `main.c`: allocate tracer memory, call `fun_trace_init` before model load, call `fun_trace_report` before model_free

## 5. gpt-demo: replace demo_time_now with fun_timing

- [x] 5.1 Replace `#include "timing.h"` with `#include "fundamental/timing/timing.h"` in main.c
- [x] 5.2 Replace `demo_time_now()` calls with `fun_timing_now_ns()` / 1e9 conversion in main.c
- [x] 5.3 Delete `demos/gpt-demo/timing.h` and `demos/gpt-demo/arch/timing/` directory

## 6. Build scripts

- [x] 6.1 Update `demos/gpt-demo/build-windows-amd64.bat` — add `../../src/trace/trace.c`, `../../arch/timing/windows-amd64/timing.c`, remove demo timing arch file
- [x] 6.2 Update `demos/gpt-demo/build-linux-amd64.sh` — add `../../src/trace/trace.c`, `../../arch/timing/linux-amd64/timing.c`, remove demo timing arch file

## 7. Validation

- [x] 7.1 Build and run gpt-demo, verify trace report prints after generation with per-phase breakdown
- [x] 7.2 Verify coordination overhead = inference_total - sum(all_fn_times) is computed and printed
- [x] 7.3 Verify single-threaded and multi-threaded produce identical trace reports (same call counts, timings differ)
- [x] 7.4 Verify no regressions — single-threaded output matches baseline
- [x] 7.5 Run `run-tests-windows-amd64.bat` to verify no regressions in existing modules
