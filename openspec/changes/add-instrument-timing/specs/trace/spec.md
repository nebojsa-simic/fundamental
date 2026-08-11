# Specifications

## ADDED Requirements

### Requirement: Caller queries memory required for tracer

The system SHALL provide `fun_trace_memory_required(max_spans)` that returns the number of bytes needed to initialize a tracer with the given span capacity.

#### Scenario: Returns positive size for valid parameter

- **WHEN** `fun_trace_memory_required(100)` is called
- **THEN** the return value SHALL be greater than 0

#### Scenario: Increasing spans increases required memory

- **WHEN** `fun_trace_memory_required(A)` and `fun_trace_memory_required(A*2)` are compared
- **THEN** the larger max_spans SHALL require at least as much memory

### Requirement: Tracer is initialized in caller-provided memory

The system SHALL provide `fun_trace_init(memory, memory_size, max_spans)` that initializes the global tracer in the caller-provided buffer. The memory SHALL be at least `fun_trace_memory_required(max_spans)` bytes. After init, all span operations SHALL be active. Before init, all operations SHALL be no-ops.

#### Scenario: Init activates tracing

- **WHEN** `fun_trace_init(buf, size, 100)` is called
- **THEN** subsequent `fun_trace_span_begin`/`end` calls SHALL accumulate timing data

#### Scenario: Insufficient memory is undefined

- **WHEN** `fun_trace_init` is called with `memory_size` smaller than `fun_trace_memory_required(max_spans)`
- **THEN** behavior is undefined (caller contract)

### Requirement: Tracer is destroyed by nulling global pointer

The system SHALL provide `fun_trace_destroy()` that nulls the global tracer pointer. After destroy, all span operations SHALL become no-ops. The function SHALL NOT free the memory buffer passed to init — the caller owns it.

#### Scenario: Destroy deactivates tracing

- **WHEN** `fun_trace_destroy()` is called after `fun_trace_init`
- **THEN** subsequent `fun_trace_span_begin` calls SHALL return immediately without side effects

#### Scenario: Re-init after destroy

- **WHEN** `fun_trace_init` is called after `fun_trace_destroy`
- **THEN** tracing SHALL be active again

### Requirement: Span names are registered with numeric IDs

The system SHALL provide `fun_trace_span_register(name)` that returns a `FunSpanId` for the given name. If the name has already been registered, the same ID SHALL be returned. The name SHALL be caller-owned and SHALL outlive the tracer. Registration SHALL be O(n) where n is the number of registered spans.

#### Scenario: Register returns unique IDs for unique names

- **WHEN** `fun_trace_span_register("foo")` and `fun_trace_span_register("bar")` are called
- **THEN** the two returned FunSpanId values SHALL differ

#### Scenario: Same name returns same ID

- **WHEN** `fun_trace_span_register("foo")` is called twice
- **THEN** the same FunSpanId SHALL be returned both times

#### Scenario: Registration is a no-op when uninitialized

- **WHEN** `fun_trace_span_register("foo")` is called before `fun_trace_init`
- **THEN** the return value SHALL be 0

### Requirement: Span begin records start time per thread

The system SHALL provide `fun_trace_span_begin(id)` that records the current time via `fun_timing_now_ns()` in thread-local storage. Only one span SHALL be active per thread at a time. Beginning a new span before ending the previous one SHALL overwrite the start time. The function SHALL be a no-op when the tracer is uninitialized.

#### Scenario: Begin stores start time

- **WHEN** `fun_trace_span_begin(id)` is called with an initialized tracer
- **THEN** the current time SHALL be recorded in thread-local storage

#### Scenario: Begin is no-op when uninitialized

- **WHEN** `fun_trace_span_begin(id)` is called with a NULL global tracer
- **THEN** the call SHALL return immediately without side effects

### Requirement: Span end accumulates elapsed time atomically

The system SHALL provide `fun_trace_span_end(id)` that atomically adds the elapsed time (current time minus the thread's stored start time) to the span's `total_ns` and increments `call_count`. Atomic operations SHALL use `__atomic_fetch_add` with relaxed ordering. The function SHALL be a no-op when the tracer is uninitialized.

#### Scenario: End accumulates time

- **WHEN** `fun_trace_span_begin(id)` is followed by `fun_trace_span_end(id)` after 100ms
- **THEN** the span's `total_ns` SHALL increase by approximately 100,000,000 nanoseconds and `call_count` SHALL increase by 1

#### Scenario: Thread-safe concurrent end calls

- **WHEN** multiple threads call `fun_trace_span_end` for the same span concurrently
- **THEN** `total_ns` and `call_count` SHALL be correctly accumulated without data races

### Requirement: Span attributes are set with typed values

The system SHALL provide `fun_trace_span_attribute_i64(id, key, value)`, `fun_trace_span_attribute_f64(id, key, value)`, and `fun_trace_span_attribute_str(id, key, value)` that associate a typed key-value attribute with a span. Attributes SHALL be stored in a fixed-size per-span array (8 slots). When the same key is set multiple times, the last value SHALL win. When the array is full, subsequent attributes SHALL be silently dropped. Keys SHALL be caller-owned and SHALL outlive the tracer.

#### Scenario: Integer attribute stored

- **WHEN** `fun_trace_span_attribute_i64(id, "layer", 3)` is called
- **THEN** the span's attribute with key "layer" SHALL have value 3 (int64)

#### Scenario: Float attribute stored

- **WHEN** `fun_trace_span_attribute_f64(id, "ratio", 0.5)` is called
- **THEN** the span's attribute with key "ratio" SHALL have value 0.5 (double)

#### Scenario: Last write wins for same key

- **WHEN** `fun_trace_span_attribute_i64(id, "x", 1)` then `fun_trace_span_attribute_i64(id, "x", 2)` are called
- **THEN** the value for "x" SHALL be 2

### Requirement: Span events count discrete occurrences

The system SHALL provide `fun_trace_span_add_event(id, event_name)` that atomically increments an event counter for the span. Event names are caller-owned strings.

#### Scenario: Event increments counter

- **WHEN** `fun_trace_span_add_event(id, "cache_miss")` is called 5 times
- **THEN** the span's event count for "cache_miss" SHALL be 5

### Requirement: Report outputs formatted text table

The system SHALL provide `fun_trace_report(output_fn)` that calls the callback with formatted text lines representing a table of all registered spans with their total time, call count, average time, and percentage of wall time. The first reported span SHALL serve as the total. Attributes SHALL be printed as indented sub-rows. The function SHALL be a no-op when the tracer is uninitialized.

#### Scenario: Report produces table with spans

- **WHEN** `fun_trace_report` is called after spans have been recorded
- **THEN** the callback SHALL receive lines containing span names, total times, call counts, and percentages

#### Scenario: Report is a no-op when uninitialized

- **WHEN** `fun_trace_report` is called before `fun_trace_init`
- **THEN** the callback SHALL NOT be invoked

### Requirement: Counter adds atomic delta

The system SHALL provide `fun_trace_counter_add(id, delta)` that atomically adds a value to a span's generic counter field. This SHALL be independent of span begin/end timing.

#### Scenario: Counter accumulates

- **WHEN** `fun_trace_counter_add(id, 10)` is called 3 times
- **THEN** the counter value SHALL be 30
