#include "fundamental/console/console.h"
#include "fundamental/math/math_kernels.h"
#include "fundamental/memory/memory.h"
#include "test_harness.h"

static int g_stub_softmax_called = 0;

static void stub_softmax(float *x, size_t n)
{
	(void)x;
	(void)n;
	g_stub_softmax_called = 1;
}

static const FunMathBackend g_stub_backend = {
	.unary = NULL,
	.binary = NULL,
	.reduce = NULL,
	.matrix_vector_f32 = NULL,
	.dot_f32 = NULL,
	.rms_norm_f32 = NULL,
	.softmax_f32 = stub_softmax,
	.rotary_f32 = NULL,
};

static TestCount test_new_kernels(void)
{
	TestCount tc = math_test_count_init();
	const int n = 64;
	float *a = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *b = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *o = (float *)fun_memory_allocate(n * sizeof(float)).value;

	if (!a || !b || !o) {
		tc.failed = 1;
		goto done;
	}

	for (int i = 0; i < n; i++) {
		a[i] = (float)(i - 32);
		b[i] = (float)(i + 1) * 0.5f;
	}

	fun_math_add_f32(a, b, o, n);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] + b[i]);

	fun_math_mul_f32(a, b, o, n);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] * b[i]);

	float sum = fun_math_sum_f32(a, n);
	float ref_sum = 0.0f;
	for (int i = 0; i < n; i++)
		ref_sum += a[i];
	math_test_count_add(&tc, sum == ref_sum);

	float mx = fun_math_max_f32(a, n);
	float ref_max = a[0];
	for (int i = 1; i < n; i++)
		if (a[i] > ref_max)
			ref_max = a[i];
	math_test_count_add(&tc, mx == ref_max);

	math_test_count_add(&tc, fun_math_sum_f32(a, 0) == 0.0f);
	math_test_count_add(&tc, fun_math_max_f32(a, 0) == 0.0f);

done:
	fun_memory_free((Memory *)&a);
	fun_memory_free((Memory *)&b);
	fun_memory_free((Memory *)&o);
	return tc;
}

static TestCount test_unary_dispatch(void)
{
	TestCount tc = math_test_count_init();
	const int n = 128;
	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *got = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *ref = (float *)fun_memory_allocate(n * sizeof(float)).value;

	if (!x || !got || !ref) {
		tc.failed = 1;
		goto done;
	}

	uint32_t rng = _math_test_lcg_seed(7);
	for (int i = 0; i < n; i++)
		x[i] = _math_test_lcg_float(&rng, -4.0f, 4.0f);

	FunMathOp ops[] = { FUN_MATH_OP_SQRT,	 FUN_MATH_OP_EXP, FUN_MATH_OP_LOG,
						FUN_MATH_OP_SIN,	 FUN_MATH_OP_COS, FUN_MATH_OP_TANH,
						FUN_MATH_OP_SIGMOID, FUN_MATH_OP_SILU };
	float (*sfns[])(float) = { fun_math_sqrt,	 fun_math_exp, fun_math_log,
							   fun_math_sin,	 fun_math_cos, fun_math_tanh,
							   fun_math_sigmoid, fun_math_silu };
	const int n_ops = 8;

	FunMathTensor in = { x, (size_t)n, 1 };
	FunMathTensor out = { got, (size_t)n, 1 };

	for (int k = 0; k < n_ops; k++) {
		fun_math_unary(ops[k], FUN_MATH_DTYPE_F32, &in, &out);
		for (int i = 0; i < n; i++) {
			/* log is undefined for non-positive inputs; skip those */
			if (ops[k] == FUN_MATH_OP_LOG && x[i] <= 0.0f)
				continue;
			ref[i] = sfns[k](x[i]);
			int ok;
			if (_math_test_float_is_nan(got[i]) ||
				_math_test_float_is_nan(ref[i]))
				ok = _math_test_float_is_nan(got[i]) ==
					 _math_test_float_is_nan(ref[i]);
			else
				ok = _math_test_check_float(got[i], ref[i], 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
		}
	}

	/* NaN propagation: a NaN input must produce a NaN output */
	{
		float nan = _math_test_make_nan();
		float nanin[1] = { nan };
		float nanout[1] = { 0.0f };
		FunMathTensor nin = { nanin, 1, 1 };
		FunMathTensor nout = { nanout, 1, 1 };
		fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F32, &nin, &nout);
		math_test_count_add(&tc, _math_test_float_is_nan(nanout[0]));
	}

done:
	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&got);
	fun_memory_free((Memory *)&ref);
	return tc;
}

static TestCount test_binary_reduce_dispatch(void)
{
	TestCount tc = math_test_count_init();
	const int n = 64;
	float *a = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *b = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *o = (float *)fun_memory_allocate(n * sizeof(float)).value;

	if (!a || !b || !o) {
		tc.failed = 1;
		goto done;
	}

	for (int i = 0; i < n; i++) {
		a[i] = (float)(i + 1);
		b[i] = (float)(i + 2);
	}

	FunMathTensor ta = { a, (size_t)n, 1 };
	FunMathTensor tb = { b, (size_t)n, 1 };
	FunMathTensor to = { o, (size_t)n, 1 };

	fun_math_binary(FUN_MATH_OP_ADD, FUN_MATH_DTYPE_F32, &ta, &tb, &to);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] + b[i]);

	fun_math_binary(FUN_MATH_OP_MUL, FUN_MATH_DTYPE_F32, &ta, &tb, &to);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] * b[i]);

	float sum_out = 0.0f;
	fun_math_reduce(FUN_MATH_OP_SUM, FUN_MATH_DTYPE_F32, &ta, &sum_out);
	float ref_sum = fun_math_sum_f32(a, n);
	math_test_count_add(&tc, sum_out == ref_sum);

	float max_out = 0.0f;
	fun_math_reduce(FUN_MATH_OP_MAX, FUN_MATH_DTYPE_F32, &ta, &max_out);
	math_test_count_add(&tc, max_out == fun_math_max_f32(a, n));

	/* zero-length reduction writes 0.0 for both sum and max */
	{
		FunMathTensor zt = { a, 0, 1 };
		float zsum = 1.0f;
		float zmax = 1.0f;
		fun_math_reduce(FUN_MATH_OP_SUM, FUN_MATH_DTYPE_F32, &zt, &zsum);
		fun_math_reduce(FUN_MATH_OP_MAX, FUN_MATH_DTYPE_F32, &zt, &zmax);
		math_test_count_add(&tc, zsum == 0.0f);
		math_test_count_add(&tc, zmax == 0.0f);
	}

done:
	fun_memory_free((Memory *)&a);
	fun_memory_free((Memory *)&b);
	fun_memory_free((Memory *)&o);
	return tc;
}

static TestCount test_strided_dispatch(void)
{
	TestCount tc = math_test_count_init();
	const int n = 32;
	const int stride = 3;
	float *buf = (float *)fun_memory_allocate(n * stride * sizeof(float)).value;
	float *got = (float *)fun_memory_allocate(n * sizeof(float)).value;

	if (!buf || !got) {
		tc.failed = 1;
		goto done;
	}

	uint32_t rng = _math_test_lcg_seed(11);
	for (int i = 0; i < n * stride; i++)
		buf[i] = _math_test_lcg_float(&rng, -3.0f, 3.0f);

	FunMathTensor in = { buf, (size_t)n, (size_t)stride };
	FunMathTensor out = { got, (size_t)n, 1 };

	fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F32, &in, &out);
	for (int i = 0; i < n; i++) {
		float ref = fun_math_exp(buf[i * stride]);
		math_test_count_add(&tc,
							_math_test_check_float(got[i], ref, 1e-4f, 1e-3f));
	}

done:
	fun_memory_free((Memory *)&buf);
	fun_memory_free((Memory *)&got);
	return tc;
}

static TestCount test_strided_binary_reduce(void)
{
	TestCount tc = math_test_count_init();
	const int n = 32;
	const int stride = 2;
	float *abuf =
		(float *)fun_memory_allocate(n * stride * sizeof(float)).value;
	float *bbuf =
		(float *)fun_memory_allocate(n * stride * sizeof(float)).value;
	float *obuf =
		(float *)fun_memory_allocate(n * stride * sizeof(float)).value;
	float *got = (float *)fun_memory_allocate(n * sizeof(float)).value;

	if (!abuf || !bbuf || !obuf || !got) {
		tc.failed = 1;
		goto done;
	}

	uint32_t rng = _math_test_lcg_seed(13);
	for (int i = 0; i < n * stride; i++) {
		abuf[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
		bbuf[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
	}

	FunMathTensor ta = { abuf, (size_t)n, (size_t)stride };
	FunMathTensor tb = { bbuf, (size_t)n, (size_t)stride };
	FunMathTensor to = { got, (size_t)n, 1 };

	fun_math_binary(FUN_MATH_OP_ADD, FUN_MATH_DTYPE_F32, &ta, &tb, &to);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, got[i] == abuf[i * stride] + bbuf[i * stride]);

	float sum_out = 0.0f;
	fun_math_reduce(FUN_MATH_OP_SUM, FUN_MATH_DTYPE_F32, &ta, &sum_out);
	float ref_sum = 0.0f;
	for (int i = 0; i < n; i++)
		ref_sum += abuf[i * stride];
	math_test_count_add(&tc, sum_out == ref_sum);

	float max_out = 0.0f;
	fun_math_reduce(FUN_MATH_OP_MAX, FUN_MATH_DTYPE_F32, &ta, &max_out);
	float ref_max = abuf[0];
	for (int i = 1; i < n; i++)
		if (abuf[i * stride] > ref_max)
			ref_max = abuf[i * stride];
	math_test_count_add(&tc, max_out == ref_max);

	/* strided output: unary writing at out[i * out.stride] */
	FunMathTensor so = { obuf, (size_t)n, (size_t)stride };
	fun_math_unary(FUN_MATH_OP_SILU, FUN_MATH_DTYPE_F32, &ta, &so);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, obuf[i * stride] ==
									 fun_math_silu(abuf[i * stride]));

done:
	fun_memory_free((Memory *)&abuf);
	fun_memory_free((Memory *)&bbuf);
	fun_memory_free((Memory *)&obuf);
	fun_memory_free((Memory *)&got);
	return tc;
}

static TestCount test_structured_wrappers(void)
{
	TestCount tc = math_test_count_init();
	const size_t n = 16;
	const size_t rows = 4;
	const size_t cols = 8;

	float *w = (float *)fun_memory_allocate(rows * cols * sizeof(float)).value;
	float *xv = (float *)fun_memory_allocate(cols * sizeof(float)).value;
	float *mv1 = (float *)fun_memory_allocate(rows * sizeof(float)).value;
	float *mv2 = (float *)fun_memory_allocate(rows * sizeof(float)).value;
	float *rx = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *rw = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *rn1 = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *rn2 = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *rotx = (float *)fun_memory_allocate(2 * n * sizeof(float)).value;
	float *cosv = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *sinv = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *rt1 = (float *)fun_memory_allocate(2 * n * sizeof(float)).value;
	float *rt2 = (float *)fun_memory_allocate(2 * n * sizeof(float)).value;

	if (!w || !xv || !mv1 || !mv2 || !rx || !rw || !rn1 || !rn2 || !rotx ||
		!cosv || !sinv || !rt1 || !rt2) {
		tc.failed = 1;
		goto done;
	}

	uint32_t rng = _math_test_lcg_seed(17);
	for (size_t i = 0; i < rows * cols; i++)
		w[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
	for (size_t i = 0; i < cols; i++)
		xv[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);

	fun_math_matrix_vector_f32_dispatch(w, xv, NULL, mv1, rows, cols);
	fun_math_matrix_vector_f32(w, xv, NULL, mv2, rows, cols);
	for (size_t i = 0; i < rows; i++)
		math_test_count_add(&tc, mv1[i] == mv2[i]);

	for (size_t i = 0; i < n; i++) {
		rx[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
		rw[i] = _math_test_lcg_float(&rng, 0.5f, 1.5f);
	}
	fun_math_rms_norm_f32_dispatch(rx, rw, rn1, n, 1e-5f);
	fun_math_rms_norm_f32(rx, rw, rn2, n, 1e-5f);
	for (size_t i = 0; i < n; i++)
		math_test_count_add(&tc, rn1[i] == rn2[i]);

	for (size_t i = 0; i < 2 * n; i++)
		rotx[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
	for (size_t i = 0; i < n; i++) {
		cosv[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
		sinv[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
	}
	fun_math_rotary_f32_dispatch(rotx, cosv, sinv, rt1, 2, n / 2);
	fun_math_rotary_f32(rotx, cosv, sinv, rt2, 2, n / 2);
	for (size_t i = 0; i < 2 * n; i++)
		math_test_count_add(&tc, rt1[i] == rt2[i]);

done:
	fun_memory_free((Memory *)&w);
	fun_memory_free((Memory *)&xv);
	fun_memory_free((Memory *)&mv1);
	fun_memory_free((Memory *)&mv2);
	fun_memory_free((Memory *)&rx);
	fun_memory_free((Memory *)&rw);
	fun_memory_free((Memory *)&rn1);
	fun_memory_free((Memory *)&rn2);
	fun_memory_free((Memory *)&rotx);
	fun_memory_free((Memory *)&cosv);
	fun_memory_free((Memory *)&sinv);
	fun_memory_free((Memory *)&rt1);
	fun_memory_free((Memory *)&rt2);
	return tc;
}

static TestCount test_backend_swap(void)
{
	TestCount tc = math_test_count_init();

	g_stub_softmax_called = 0;
	const FunMathBackend *before = fun_math_backend_get();
	math_test_count_add(&tc, before != NULL);

	fun_math_backend_set(&g_stub_backend);
	math_test_count_add(&tc, fun_math_backend_get() == &g_stub_backend);

	float x[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
	fun_math_softmax_f32_dispatch(x, 4);
	math_test_count_add(&tc, g_stub_softmax_called == 1);

	/* structured entries not provided by the stub fall back to the CPU */
	float a[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float b[4] = { 2.0f, 3.0f, 4.0f, 5.0f };
	float d = fun_math_dot_f32_dispatch(a, b, 4);
	math_test_count_add(&tc, d == 14.0f);

	/* generic entries left NULL by the stub also fall back to the CPU */
	float e[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
	float eout[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	FunMathTensor ein = { e, 4, 1 };
	FunMathTensor eot = { eout, 4, 1 };
	fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F32, &ein, &eot);
	math_test_count_add(&tc, eout[0] == fun_math_exp(1.0f));
	math_test_count_add(&tc, eout[3] == fun_math_exp(4.0f));

	fun_math_backend_set(NULL);
	math_test_count_add(&tc, fun_math_backend_get() == before);

	return tc;
}

static TestCount test_reserved_dtype(void)
{
	TestCount tc = math_test_count_init();
	float x[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
	float y[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	FunMathTensor in = { x, 4, 1 };
	FunMathTensor out = { y, 4, 1 };
	fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F16, &in, &out);
	for (int i = 0; i < 4; i++)
		math_test_count_add(&tc, y[i] == 0.0f);

	return tc;
}

TestCount test_dispatch(void)
{
	TestCount total = math_test_count_init();

	fun_console_write_line("");
	fun_console_write("    new kernels: ");
	TestCount tc = test_new_kernels();
	math_test_count_merge(&total, tc);

	fun_console_write("    unary dispatch: ");
	tc = test_unary_dispatch();
	math_test_count_merge(&total, tc);

	fun_console_write("    binary/reduce dispatch: ");
	tc = test_binary_reduce_dispatch();
	math_test_count_merge(&total, tc);

	fun_console_write("    strided dispatch: ");
	tc = test_strided_dispatch();
	math_test_count_merge(&total, tc);

	fun_console_write("    strided binary/reduce: ");
	tc = test_strided_binary_reduce();
	math_test_count_merge(&total, tc);

	fun_console_write("    structured wrappers: ");
	tc = test_structured_wrappers();
	math_test_count_merge(&total, tc);

	fun_console_write("    backend swap: ");
	tc = test_backend_swap();
	math_test_count_merge(&total, tc);

	fun_console_write("    reserved dtype: ");
	tc = test_reserved_dtype();
	math_test_count_merge(&total, tc);

	char _buf[32];
	fun_string_from_int(total.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write("/");
	fun_string_from_int(total.passed + total.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	if (total.failed) {
		fun_console_write(" (");
		fun_string_from_int(total.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" FAILED)");
	}
	fun_console_write_line("");

	return total;
}
