/*
 * trace-demo: a parallel request-processing pipeline measured with fun_trace.
 *
 * A pool of workers processes a batch of requests. Each request runs two
 * CPU-bound phases (decode + transform) while a per-thread start timestamp
 * records wall time via the trace module's thread-local storage.
 *
 * The demo is built freestanding (-nostdlib): no libc, no CRT, no libgcc.
 * That is deliberate. If src/trace/trace.c ever reintroduces _Thread_local
 * (or __thread on MinGW), linking fails with an undefined reference to
 * __emutls_get_address because libgcc's emulated-TLS runtime is absent.
 */

#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "fundamental/thread_pool/thread_pool.h"
#include "fundamental/timing/timing.h"
#include "fundamental/trace/trace.h"

#define WORKERS 4
#define REQUESTS 16
#define DECODE_NS 3000000ULL
#define TRANSFORM_NS 2000000ULL
#define BACKOFF_NS 50000ULL

typedef struct {
	FunSpanId span;
	int index;
} JobArg;

static FunSpanId g_request_span;

static void busy_wait(uint64_t ns)
{
	uint64_t t0 = fun_timing_now_ns();
	while (fun_timing_now_ns() - t0 < ns)
		;
}

static void job_fn(void *data)
{
	JobArg *a = (JobArg *)data;

	fun_trace_span_begin(a->span);
	fun_trace_span_attribute_i64(a->span, "worker", a->index);

	busy_wait(DECODE_NS);
	fun_trace_span_add_event(a->span, "decode");
	busy_wait(TRANSFORM_NS);
	fun_trace_span_add_event(a->span, "transform");

	fun_trace_span_end(a->span);
	fun_trace_counter_add(a->span, 1);
}

static void report_line(const char *line)
{
	fun_console_write_line(line);
}

int cli_main(int argc, const char **argv)
{
	(void)argc;
	(void)argv;

	fun_console_write_line("trace-demo: parallel request pipeline");
	fun_console_write_line(
		"workers=4 requests=16 (decode 3ms + transform 2ms)");
	fun_console_write_line("");

	size_t sz = fun_trace_memory_required(8);
	Memory m = fun_memory_allocate(sz).value;
	if (!m)
		return 1;
	fun_trace_init(m, sz, 8);

	FunSpanId total = fun_trace_span_register("total");
	g_request_span = fun_trace_span_register("request");

	fun_trace_span_begin(total);

	ThreadPool pool = NULL;
	voidResult pr = fun_thread_pool_create(WORKERS, &pool);
	if (fun_error_is_error(pr.error)) {
		fun_console_error_line("thread pool create failed");
		fun_trace_destroy();
		fun_memory_free(&m);
		return 1;
	}

	JobArg args[REQUESTS];
	for (int i = 0; i < REQUESTS; i++) {
		args[i].span = g_request_span;
		args[i].index = i;
		WorkItem item = { &args[i], sizeof(args[i]), job_fn };
		while (fun_error_is_error(fun_thread_pool_submit(pool, &item).error))
			busy_wait(BACKOFF_NS);
	}

	fun_thread_pool_destroy(pool);
	fun_trace_span_end(total);

	fun_trace_report(report_line);

	fun_trace_destroy();
	fun_memory_free(&m);
	return 0;
}
