#include "fundamental/compute/compute.h"
#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"

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

/* ── Test 5.2: single task, no deps ── */

typedef struct {
	int *out;
	int value;
} CtxSingle;

static void _single_fn(void *ctx)
{
	CtxSingle *c = (CtxSingle *)ctx;
	*c->out = c->value;
}

static void test_single_task(void)
{
	fun_console_write_line("5.2 single task, no dependencies");

	int result = 0;
	CtxSingle ctx = { &result, 42 };

	size_t bytes = fun_compute_graph_memory_required(4, 4, 0);
	Memory m = fun_memory_allocate(bytes).value;
	FunComputeGraph g = fun_compute_graph_init(m, bytes, 4, 4, 0);

	FunComputeTask t;
	fun_compute_graph_add_task(g, &t, _single_fn, &ctx, NULL, NULL);

	fun_compute_graph_submit(g, NULL);
	fun_compute_graph_wait(g);

	check(result == 42, "writes result to shared buffer");

	fun_compute_graph_destroy(g);
	fun_memory_free(&m);
}

/* ── Test 5.3: chain A → B → C ── */

typedef struct {
	int *buffer;
	int add;
} CtxAdd;

static void _add_fn(void *ctx)
{
	CtxAdd *c = (CtxAdd *)ctx;
	*c->buffer += c->add;
}

static void test_chain(void)
{
	fun_console_write_line("5.3 chain A → B → C");

	int buf = 10;
	CtxAdd a_ctx = { &buf, 1 };
	CtxAdd b_ctx = { &buf, 2 };
	CtxAdd c_ctx = { &buf, 3 };

	size_t bytes = fun_compute_graph_memory_required(4, 4, 0);
	Memory m = fun_memory_allocate(bytes).value;
	FunComputeGraph g = fun_compute_graph_init(m, bytes, 4, 4, 0);

	FunComputeTask ta, tb, tc;
	fun_compute_graph_add_task(g, &ta, _add_fn, &a_ctx, NULL, NULL);
	fun_compute_graph_add_task(g, &tb, _add_fn, &b_ctx, NULL, NULL);
	fun_compute_graph_add_task(g, &tc, _add_fn, &c_ctx, NULL, NULL);

	fun_compute_task_depends_on(g, &tb, &ta);
	fun_compute_task_depends_on(g, &tc, &tb);

	fun_compute_graph_submit(g, NULL);
	fun_compute_graph_wait(g);

	check(buf == 16, "C sees B's output (10+1+2+3=16)");

	fun_compute_graph_destroy(g);
	fun_memory_free(&m);
}

/* ── Test 5.4: parallel roots ── */

typedef struct {
	int *a_out, *b_out;
	int value_a, value_b;
} CtxParallelRoot;

static void _set_a_fn(void *ctx)
{
	CtxParallelRoot *c = (CtxParallelRoot *)ctx;
	*c->a_out = c->value_a;
}

static void _set_b_fn(void *ctx)
{
	CtxParallelRoot *c = (CtxParallelRoot *)ctx;
	c->b_out[0] = c->value_b;
}

typedef struct {
	int *a_val;
	int *b_val;
	int *out;
} CtxCombine;

static void _combine_fn(void *ctx)
{
	CtxCombine *c = (CtxCombine *)ctx;
	*c->out = *c->a_val + *c->b_val;
}

static void test_parallel_roots(void)
{
	fun_console_write_line("5.4 parallel roots A,B → C");

	int a = 0, b = 0, result = 0;
	CtxParallelRoot pr = { &a, &b, 10, 20 };
	CtxCombine cc = { &a, &b, &result };

	size_t bytes = fun_compute_graph_memory_required(4, 4, 0);
	Memory m = fun_memory_allocate(bytes).value;
	FunComputeGraph g = fun_compute_graph_init(m, bytes, 4, 4, 0);

	FunComputeTask ta, tb, tc;
	fun_compute_graph_add_task(g, &ta, _set_a_fn, &pr, NULL, NULL);
	fun_compute_graph_add_task(g, &tb, _set_b_fn, &pr, NULL, NULL);
	fun_compute_graph_add_task(g, &tc, _combine_fn, &cc, NULL, NULL);

	fun_compute_task_depends_on(g, &tc, &ta);
	fun_compute_task_depends_on(g, &tc, &tb);

	fun_compute_graph_submit(g, NULL);
	fun_compute_graph_wait(g);

	check(a == 10, "task A executed");
	check(b == 20, "task B executed");
	check(result == 30, "C sees both outputs (10+20=30)");

	fun_compute_graph_destroy(g);
	fun_memory_free(&m);
}

/* ── Test 5.5: multiple submit/wait ── */

typedef struct {
	int *counter;
	int increment;
} CtxIncrement;

typedef struct {
	int *position;
	int *value_at_pos;
} SctxIncrement;

static void _increment_bind(void *task_ctx, void *submit_ctx)
{
	CtxIncrement *c = (CtxIncrement *)task_ctx;
	SctxIncrement *s = (SctxIncrement *)submit_ctx;
	c->counter = s->position;
	c->increment = s->value_at_pos[0];
}

static void _increment_fn(void *ctx)
{
	CtxIncrement *c = (CtxIncrement *)ctx;
	*c->counter += c->increment;
}

static void test_multiple_submit(void)
{
	fun_console_write_line("5.5 multiple submit/wait cycles");

	int counter = 0;
	int vals[] = { 5, 10, 15 };
	CtxIncrement ctx;

	size_t bytes = fun_compute_graph_memory_required(2, 2, 0);
	Memory m = fun_memory_allocate(bytes).value;
	FunComputeGraph g = fun_compute_graph_init(m, bytes, 2, 2, 0);

	FunComputeTask t;
	fun_compute_graph_add_task(g, &t, _increment_fn, &ctx,
				   _increment_bind, NULL);

	SctxIncrement s1 = { &counter, &vals[0] };
	fun_compute_graph_submit(g, &s1);
	fun_compute_graph_wait(g);
	check(counter == 5, "first submit: 0+5=5");

	SctxIncrement s2 = { &counter, &vals[1] };
	fun_compute_graph_submit(g, &s2);
	fun_compute_graph_wait(g);
	check(counter == 15, "second submit: 5+10=15");

	SctxIncrement s3 = { &counter, &vals[2] };
	fun_compute_graph_submit(g, &s3);
	fun_compute_graph_wait(g);
	check(counter == 30, "third submit: 15+15=30");

	fun_compute_graph_destroy(g);
	fun_memory_free(&m);
}

/* ── Test 5.6: destroy calls destructors ── */

static int dtor_count = 0;

static void _test_destroy_fn(void *ctx) { (void)ctx; }
static void _test_dtor(void *ctx) { (void)ctx; dtor_count++; }

static void test_destroy(void)
{
	fun_console_write_line("5.6 destroy calls destructors");

	dtor_count = 0;
	int ctx_buf[3];

	size_t bytes = fun_compute_graph_memory_required(4, 4, 0);
	Memory m = fun_memory_allocate(bytes).value;
	FunComputeGraph g = fun_compute_graph_init(m, bytes, 4, 4, 0);

	FunComputeTask t0, t1, t2;
	fun_compute_graph_add_task(g, &t0, _test_destroy_fn, &ctx_buf[0],
				   NULL, _test_dtor);
	fun_compute_graph_add_task(g, &t1, _test_destroy_fn, &ctx_buf[1],
				   NULL, _test_dtor);
	fun_compute_graph_add_task(g, &t2, _test_destroy_fn, &ctx_buf[2],
				   NULL, NULL);

	fun_compute_graph_destroy(g);
	check(dtor_count == 2, "two destructors called, NULL skipped");
	fun_memory_free(&m);
}

/* ── Test 5.7: multi-threaded matches single-threaded ── */

typedef struct {
	int *out;
	int value;
} CtxThreadedSet;

typedef struct {
	int *a_out;
	int *b_out;
	int *result;
} CtxThreadedSum;

static void _threaded_set_fn(void *ctx)
{
	CtxThreadedSet *c = (CtxThreadedSet *)ctx;
	*c->out = c->value;
}

static void _threaded_sum_fn(void *ctx)
{
	CtxThreadedSum *c = (CtxThreadedSum *)ctx;
	*c->result = *c->a_out + *c->b_out;
}

static void test_multi_threaded(void)
{
	fun_console_write_line("5.7 multi-threaded matches single-threaded");

	int a_st = 0, b_st = 0, result_st = 0;
	int a_mt = 0, b_mt = 0, result_mt = 0;

	/* single-threaded run */
	{
		size_t bytes = fun_compute_graph_memory_required(4, 4, 0);
		Memory m = fun_memory_allocate(bytes).value;
		FunComputeGraph g = fun_compute_graph_init(m, bytes, 4, 4, 0);

		CtxThreadedSet ctx_a = { &a_st, 10 };
		CtxThreadedSet ctx_b = { &b_st, 20 };
		CtxThreadedSum ctx_s = { &a_st, &b_st, &result_st };

		FunComputeTask ta, tb, ts;
		fun_compute_graph_add_task(g, &ta, _threaded_set_fn, &ctx_a,
					   NULL, NULL);
		fun_compute_graph_add_task(g, &tb, _threaded_set_fn, &ctx_b,
					   NULL, NULL);
		fun_compute_graph_add_task(g, &ts, _threaded_sum_fn, &ctx_s,
					   NULL, NULL);
		fun_compute_task_depends_on(g, &ts, &ta);
		fun_compute_task_depends_on(g, &ts, &tb);

		fun_compute_graph_submit(g, NULL);
		fun_compute_graph_wait(g);
		fun_compute_graph_destroy(g);
		fun_memory_free(&m);
	}

	/* multi-threaded run (2 workers) */
	{
		size_t bytes = fun_compute_graph_memory_required(4, 4, 2);
		Memory m = fun_memory_allocate(bytes).value;
		FunComputeGraph g = fun_compute_graph_init(m, bytes, 4, 4, 2);

		CtxThreadedSet ctx_a = { &a_mt, 10 };
		CtxThreadedSet ctx_b = { &b_mt, 20 };
		CtxThreadedSum ctx_s = { &a_mt, &b_mt, &result_mt };

		FunComputeTask ta, tb, ts;
		fun_compute_graph_add_task(g, &ta, _threaded_set_fn, &ctx_a,
					   NULL, NULL);
		fun_compute_graph_add_task(g, &tb, _threaded_set_fn, &ctx_b,
					   NULL, NULL);
		fun_compute_graph_add_task(g, &ts, _threaded_sum_fn, &ctx_s,
					   NULL, NULL);
		fun_compute_task_depends_on(g, &ts, &ta);
		fun_compute_task_depends_on(g, &ts, &tb);

		fun_compute_graph_submit(g, NULL);
		fun_compute_graph_wait(g);
		fun_compute_graph_destroy(g);
		fun_memory_free(&m);
	}

	check(result_st == result_mt,
	      "single-threaded and multi-threaded produce same result");
	check(result_mt == 30, "result is 10+20=30");
}

/* ── Test 5.8: bind called once per submit, before fn ── */

static int bind_count = 0;
static int fn_ran = 0;

static void _bind_counter_fn(void *task_ctx, void *submit_ctx)
{
	(void)task_ctx;
	(void)submit_ctx;
	bind_count++;
}

static void _fn_marker(void *ctx)
{
	(void)ctx;
	fn_ran = 1;
}

static void test_bind(void)
{
	fun_console_write_line("5.8 bind called once per submit");

	int ctx_val = 0;
	bind_count = 0;
	fn_ran = 0;

	size_t bytes = fun_compute_graph_memory_required(4, 4, 0);
	Memory m = fun_memory_allocate(bytes).value;
	FunComputeGraph g = fun_compute_graph_init(m, bytes, 4, 4, 0);

	FunComputeTask t_nb, t_b;
	fun_compute_graph_add_task(g, &t_nb, _fn_marker, &ctx_val, NULL, NULL);
	fun_compute_graph_add_task(g, &t_b, _fn_marker, &ctx_val,
				   _bind_counter_fn, NULL);

	fun_compute_task_depends_on(g, &t_b, &t_nb);

	/* First submit */
	fun_compute_graph_submit(g, NULL);
	/* After submit returned, bind must have been called already (before fn) */
	check(bind_count == 1, "bind called during submit");
	check(fn_ran == 0, "fn NOT called before wait");
	fun_compute_graph_wait(g);

	/* Second submit */
	bind_count = 0;
	fn_ran = 0;
	fun_compute_graph_submit(g, NULL);
	check(bind_count == 1, "bind called again on second submit");
	fun_compute_graph_wait(g);

	fun_compute_graph_destroy(g);
	fun_memory_free(&m);
}

int main(void)
{
	test_single_task();
	test_chain();
	test_parallel_roots();
	test_multiple_submit();
	test_destroy();
	test_multi_threaded();
	test_bind();

	char buf[32];
	fun_string_from_int(failed, 10, buf, 32);
	fun_console_write("Failures: ");
	fun_console_write_line(buf);

	return failed ? 1 : 0;
}
