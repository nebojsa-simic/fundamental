#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "test_harness.h"

static __inline__ uint64_t _math_test_rdtsc(void)
{
	uint32_t lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
}

static void bench_scalar(const char *name, float (*fn)(float), int n, int reps)
{
	uint64_t best = ~0ULL;

	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
	if (!x || !out)
		return;

	for (int i = 0; i < n; i++)
		x[i] = (float)(i % 256) * 0.1f - 12.8f;

	for (int r = 0; r < reps; r++) {
		uint64_t t0 = _math_test_rdtsc();
		for (int i = 0; i < n; i++)
			out[i] = fn(x[i]);
		uint64_t t1 = _math_test_rdtsc();
		uint64_t elapsed = t1 - t0;
		if (elapsed < best)
			best = elapsed;
	}

	fun_console_write("    ");
	fun_console_write(name);
	fun_console_write(": ");
	char _buf[64];
	fun_string_from_double((double)best / (double)n, 2, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cyc/el (");
	fun_string_from_int((int64_t)best, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cycles / ");
	fun_string_from_int(n, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(")");

	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&out);
}

static void bench_vector(const char *name,
						 void (*fn)(const float *, float *, size_t), int n,
						 int reps)
{
	uint64_t best = ~0ULL;

	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
	if (!x || !out)
		return;

	for (int i = 0; i < n; i++)
		x[i] = (float)(i % 256) * 0.1f - 12.8f;

	for (int r = 0; r < reps; r++) {
		uint64_t t0 = _math_test_rdtsc();
		fn(x, out, n);
		uint64_t t1 = _math_test_rdtsc();
		uint64_t elapsed = t1 - t0;
		if (elapsed < best)
			best = elapsed;
	}

	fun_console_write("    ");
	fun_console_write(name);
	fun_console_write(": ");
	char _buf[64];
	fun_string_from_double((double)best / (double)n, 2, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cyc/el (");
	fun_string_from_int((int64_t)best, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cycles / ");
	fun_string_from_int(n, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(")");

	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&out);
}

static void bench_vector_swiglu(const char *name,
								void (*fn)(const float *, const float *,
										   float *, size_t),
								int n, int reps)
{
	uint64_t best = ~0ULL;

	float *gate = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *up = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
	if (!gate || !up || !out)
		goto done;

	for (int i = 0; i < n; i++) {
		gate[i] = (float)(i % 256) * 0.1f - 12.8f;
		up[i] = (float)(i % 128) * 0.05f - 3.2f;
	}

	for (int r = 0; r < reps; r++) {
		uint64_t t0 = _math_test_rdtsc();
		fn(gate, up, out, n);
		uint64_t t1 = _math_test_rdtsc();
		uint64_t elapsed = t1 - t0;
		if (elapsed < best)
			best = elapsed;
	}

	fun_console_write("    ");
	fun_console_write(name);
	fun_console_write(": ");
	char _buf[64];
	fun_string_from_double((double)best / (double)n, 2, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cyc/el (");
	fun_string_from_int((int64_t)best, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cycles / ");
	fun_string_from_int(n, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(")");

done:
	fun_memory_free((Memory *)&gate);
	fun_memory_free((Memory *)&up);
	fun_memory_free((Memory *)&out);
}

static void bench_vector_rms(const char *name,
							 void (*fn)(const float *, const float *, float *,
										size_t, float),
							 int n, int reps, float eps)
{
	uint64_t best = ~0ULL;

	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *w = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
	if (!x || !w || !out)
		goto done;

	for (int i = 0; i < n; i++) {
		x[i] = (float)(i % 256) * 0.1f - 12.8f;
		w[i] = 1.0f;
	}

	for (int r = 0; r < reps; r++) {
		uint64_t t0 = _math_test_rdtsc();
		fn(x, w, out, n, eps);
		uint64_t t1 = _math_test_rdtsc();
		uint64_t elapsed = t1 - t0;
		if (elapsed < best)
			best = elapsed;
	}

	fun_console_write("    ");
	fun_console_write(name);
	fun_console_write(": ");
	char _buf[64];
	fun_string_from_double((double)best / (double)n, 2, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cyc/el (");
	fun_string_from_int((int64_t)best, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cycles / ");
	fun_string_from_int(n, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(")");

done:
	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&w);
	fun_memory_free((Memory *)&out);
}

static void bench_dot(const char *name, int n, int reps)
{
	uint64_t best = ~0ULL;

	float *a = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *b = (float *)fun_memory_allocate(n * sizeof(float)).value;
	if (!a || !b)
		return;

	for (int i = 0; i < n; i++) {
		a[i] = (float)(i % 256) * 0.1f - 12.8f;
		b[i] = (float)(i % 128) * 0.05f - 3.2f;
	}

	for (int r = 0; r < reps; r++) {
		uint64_t t0 = _math_test_rdtsc();
		fun_math_dot_f32(a, b, n);
		uint64_t t1 = _math_test_rdtsc();
		uint64_t elapsed = t1 - t0;
		if (elapsed < best)
			best = elapsed;
	}

	fun_console_write("    ");
	fun_console_write(name);
	fun_console_write(": ");
	char _buf[64];
	fun_string_from_double((double)best / (double)n, 2, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cyc/el (");
	fun_string_from_int((int64_t)best, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cycles / ");
	fun_string_from_int(n, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(")");

	fun_memory_free((Memory *)&a);
	fun_memory_free((Memory *)&b);
}

static void bench_mat_vec(const char *name, int rows, int cols, int reps)
{
	uint64_t best = ~0ULL;

	float *w =
		(float *)fun_memory_allocate((size_t)rows * cols * sizeof(float)).value;
	float *x = (float *)fun_memory_allocate(cols * sizeof(float)).value;
	float *bias = (float *)fun_memory_allocate(rows * sizeof(float)).value;
	float *out = (float *)fun_memory_allocate(rows * sizeof(float)).value;
	if (!w || !x || !bias || !out)
		goto done;

	for (int i = 0; i < rows * cols; i++)
		w[i] = (float)(i % 256) * 0.1f - 12.8f;
	for (int i = 0; i < cols; i++)
		x[i] = (float)(i % 128) * 0.05f - 3.2f;
	for (int i = 0; i < rows; i++)
		bias[i] = 0.5f;

	for (int r = 0; r < reps; r++) {
		uint64_t t0 = _math_test_rdtsc();
		fun_math_matrix_vector_f32(w, x, bias, out, rows, cols);
		uint64_t t1 = _math_test_rdtsc();
		uint64_t elapsed = t1 - t0;
		if (elapsed < best)
			best = elapsed;
	}

	fun_console_write("    ");
	fun_console_write(name);
	fun_console_write(": ");
	char _buf[64];
	fun_string_from_double((double)best / (double)((size_t)rows * cols), 2,
						   _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cyc/el (");
	fun_string_from_int((int64_t)best, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" cycles / ");
	fun_string_from_int(rows * cols, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(")");

done:
	fun_memory_free((Memory *)&w);
	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&bias);
	fun_memory_free((Memory *)&out);
}

static void bench_noop(void)
{
	int n = 65536;
	int reps = 100;
	uint64_t best = ~0ULL;

	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
	if (!x || !out)
		return;

	for (int i = 0; i < n; i++)
		x[i] = (float)i;

	for (int r = 0; r < reps; r++) {
		uint64_t t0 = _math_test_rdtsc();
		_math_bench_noop(x, out, n);
		uint64_t t1 = _math_test_rdtsc();
		uint64_t elapsed = t1 - t0;
		if (elapsed < best)
			best = elapsed;
	}

	fun_console_write("    ");
	fun_console_write("noop");
	fun_console_write(": ");
	char _buf[64];
	fun_string_from_double((double)best / (double)n, 2, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(" cyc/el (loop overhead)");

	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&out);
}

TestCount test_performance(void)
{
	TestCount tc = math_test_count_init();

	fun_console_write_line("");
	fun_console_write_line("    Scalar (n=1024, best of 100000):");
	bench_scalar("sqrt", fun_math_sqrt, 1024, 100000);
	bench_scalar("exp", fun_math_exp, 1024, 100000);
	bench_scalar("log", fun_math_log, 1024, 100000);
	bench_scalar("sin", fun_math_sin, 1024, 100000);
	bench_scalar("cos", fun_math_cos, 1024, 100000);
	bench_scalar("tanh", fun_math_tanh, 1024, 100000);
	bench_scalar("sigmoid", fun_math_sigmoid, 1024, 100000);
	bench_scalar("silu", fun_math_silu, 1024, 100000);

	fun_console_write_line("");
	fun_console_write_line("    Vector (n=65536, best of 10000):");
	bench_noop();
	bench_vector("silu_f32", fun_math_silu_f32, 65536, 10000);
	bench_vector_rms("rms_norm", fun_math_rms_norm_f32, 65536, 1000, 1e-5f);
	bench_vector_swiglu("swiglu_f32", fun_math_swiglu_f32, 65536, 1000);
	bench_vector("exp_f32", fun_math_exp_f32, 65536, 1000);
	bench_vector("log_f32", fun_math_log_f32, 65536, 1000);
	bench_vector("sin_f32", fun_math_sin_f32, 65536, 1000);
	bench_vector("cos_f32", fun_math_cos_f32, 65536, 1000);
	bench_dot("dot_f32", 65536, 1000);
	bench_mat_vec("mat_vec", 64, 1024, 1000);

	{
		float *x = (float *)fun_memory_allocate(64 * sizeof(float)).value;
		float *xcopy = (float *)fun_memory_allocate(64 * sizeof(float)).value;
		if (x && xcopy) {
			uint64_t best = ~0ULL;
			for (int i = 0; i < 64; i++)
				x[i] = (float)(i * i) * 0.01f;

			for (int r = 0; r < 100000; r++) {
				for (int i = 0; i < 64; i++)
					xcopy[i] = x[i];
				uint64_t t0 = _math_test_rdtsc();
				fun_math_softmax_f32(xcopy, 64);
				uint64_t t1 = _math_test_rdtsc();
				if (t1 - t0 < best)
					best = t1 - t0;
			}
			fun_console_write("    ");
			fun_console_write("softmax");
			fun_console_write(": ");
			char _buf[64];
			fun_string_from_double((double)best / 64.0, 2, _buf, sizeof(_buf));
			fun_console_write(_buf);
			fun_console_write(" cyc/el (");
			fun_string_from_int((int64_t)best, 10, _buf, sizeof(_buf));
			fun_console_write(_buf);
			fun_console_write(" cycles / 64)");
			fun_console_write_line("");
		}
		fun_memory_free((Memory *)&x);
		fun_memory_free((Memory *)&xcopy);
	}

	tc.passed = 1;
	return tc;
}
