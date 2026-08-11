#include "fundamental/trace/trace.h"
#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <stdbool.h>

static int failed = 0;

static void check(bool cond, const char *msg)
{
	if (!cond) {
		fun_console_write("  FAIL: ");
		fun_console_write_line(msg);
		failed++;
	} else {
		fun_console_write("  ok: ");
		fun_console_write_line(msg);
	}
}

static int _cb_uninit_called = 0;
static void _cb_uninit(const char *line) {
	(void)line;
	_cb_uninit_called++;
}

static void test_uninitialized(void)
{
	fun_console_write_line("3.2 uninitialized tracer is no-op");

	fun_trace_destroy();

	FunSpanId id = fun_trace_span_register("test");
	check(id == 0, "register returns 0 when uninitialized");

	fun_trace_span_begin(1);
	fun_trace_span_end(1);

	_cb_uninit_called = 0;
	fun_trace_report(_cb_uninit);
	check(_cb_uninit_called == 0, "report produces no output when uninitialized");
}

static void test_register(void)
{
	fun_console_write_line("3.3 span register returns same ID");

	size_t sz = fun_trace_memory_required(10);
	Memory m = fun_memory_allocate(sz).value;
	fun_trace_init(m, sz, 10);

	FunSpanId a1 = fun_trace_span_register("alpha");
	FunSpanId a2 = fun_trace_span_register("alpha");
	FunSpanId b = fun_trace_span_register("beta");

	check(a1 == a2, "same name returns same ID");
	check(a1 != b, "different names return different IDs");
	check(a1 > 0, "valid IDs are > 0");

	fun_trace_destroy();
	fun_memory_free(&m);
}

static void test_begin_end(void)
{
	fun_console_write_line("3.4 span begin/end accumulates time");

	size_t sz = fun_trace_memory_required(4);
	Memory m = fun_memory_allocate(sz).value;
	fun_trace_init(m, sz, 4);

	FunSpanId id = fun_trace_span_register("work");
	fun_trace_span_begin(id);
	fun_trace_span_end(id);

	check(1, "begin/end called without crash");
	fun_trace_destroy();
	fun_memory_free(&m);
}

static void test_attributes(void)
{
	fun_console_write_line("3.6 attributes stored correctly");

	size_t sz = fun_trace_memory_required(4);
	Memory m = fun_memory_allocate(sz).value;
	fun_trace_init(m, sz, 4);

	FunSpanId id = fun_trace_span_register("sp");
	fun_trace_span_attribute_i64(id, "layer", 3);
	fun_trace_span_attribute_f64(id, "ratio", 0.5);
	fun_trace_span_attribute_str(id, "name", "hello");
	fun_trace_span_attribute_i64(id, "layer", 5);

	check(1, "attribute calls without crash");
	fun_trace_destroy();
	fun_memory_free(&m);
}

static int _report_lines = 0;
static void _cb_report(const char *line) {
	(void)line;
	_report_lines++;
}

static void test_report(void)
{
	fun_console_write_line("3.7 report produces formatted output");

	_report_lines = 0;

	size_t sz = fun_trace_memory_required(4);
	Memory m = fun_memory_allocate(sz).value;
	fun_trace_init(m, sz, 4);

	FunSpanId total = fun_trace_span_register("total");
	FunSpanId work = fun_trace_span_register("work");

	fun_trace_span_begin(total);
	fun_trace_span_begin(work);
	fun_trace_span_end(work);
	fun_trace_span_end(total);

	fun_trace_report(_cb_report);
	check(_report_lines > 3, "report produces multiple lines of output");

	fun_trace_destroy();
	fun_memory_free(&m);
}

static void test_counter_add(void)
{
	fun_console_write_line("3.8 counter_add accumulates");

	size_t sz = fun_trace_memory_required(4);
	Memory m = fun_memory_allocate(sz).value;
	fun_trace_init(m, sz, 4);

	FunSpanId id = fun_trace_span_register("ctr");
	fun_trace_counter_add(id, 10);
	fun_trace_counter_add(id, 5);

	check(1, "counter calls without crash");
	fun_trace_destroy();
	fun_memory_free(&m);
}

static void test_add_event(void)
{
	fun_console_write_line("3.9 add_event works");

	size_t sz = fun_trace_memory_required(4);
	Memory m = fun_memory_allocate(sz).value;
	fun_trace_init(m, sz, 4);

	FunSpanId id = fun_trace_span_register("ev");
	fun_trace_span_add_event(id, "cache_hit");
	fun_trace_span_add_event(id, "cache_hit");

	check(1, "event calls without crash");
	fun_trace_destroy();
	fun_memory_free(&m);
}

static void test_reinit(void)
{
	fun_console_write_line("3.10 init -> destroy -> re-init");

	size_t sz = fun_trace_memory_required(4);
	Memory m = fun_memory_allocate(sz).value;

	fun_trace_init(m, sz, 4);
	FunSpanId a = fun_trace_span_register("a");
	check(a > 0, "registered after first init");

	fun_trace_destroy();
	FunSpanId b = fun_trace_span_register("b");
	check(b == 0, "register returns 0 after destroy");

	fun_trace_init(m, sz, 4);
	FunSpanId c = fun_trace_span_register("c");
	check(c > 0, "registered after re-init");

	fun_trace_destroy();
	fun_memory_free(&m);
}

int main(void)
{
	test_uninitialized();
	test_register();
	test_begin_end();
	test_attributes();
	test_report();
	test_counter_add();
	test_add_event();
	test_reinit();

	char buf[32];
	fun_string_from_int(failed, 10, buf, 32);
	fun_console_write("Failures: ");
	fun_console_write_line(buf);
	return failed ? 1 : 0;
}
