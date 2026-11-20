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

static int g_stub_swiglu_called = 0;
static int g_stub_rows_dot_called = 0;

static void stub_swiglu(const float *gate, const float *up, float *out,
						size_t n)
{
	(void)gate;
	(void)up;
	(void)out;
	(void)n;
	g_stub_swiglu_called = 1;
}

static void stub_rows_dot(const float *q, const float *x, float *out,
						  size_t n_rows, size_t row_len, size_t row_stride,
						  float scale)
{
	(void)q;
	(void)x;
	(void)out;
	(void)n_rows;
	(void)row_len;
	(void)row_stride;
	(void)scale;
	g_stub_rows_dot_called = 1;
}

static const FunMathBackend g_stub_structured_backend = {
	.unary = NULL,
	.binary = NULL,
	.reduce = NULL,
	.matrix_vector_f32 = NULL,
	.dot_f32 = NULL,
	.rms_norm_f32 = NULL,
	.softmax_f32 = NULL,
	.rotary_f32 = NULL,
	.swiglu_f32 = stub_swiglu,
	.rows_dot_f32 = stub_rows_dot,
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

	fun_math_matrix_vector_f32(w, xv, NULL, mv1, rows, cols);
	fun_math_matrix_vector_f32_cpu(w, xv, NULL, mv2, rows, cols);
	for (size_t i = 0; i < rows; i++)
		math_test_count_add(&tc, mv1[i] == mv2[i]);

	for (size_t i = 0; i < n; i++) {
		rx[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
		rw[i] = _math_test_lcg_float(&rng, 0.5f, 1.5f);
	}
	fun_math_rms_norm_f32(rx, rw, rn1, n, 1e-5f);
	fun_math_rms_norm_f32_cpu(rx, rw, rn2, n, 1e-5f);
	for (size_t i = 0; i < n; i++)
		math_test_count_add(&tc, rn1[i] == rn2[i]);

	for (size_t i = 0; i < 2 * n; i++)
		rotx[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
	for (size_t i = 0; i < n; i++) {
		cosv[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
		sinv[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
	}
	fun_math_rotary_f32(rotx, cosv, sinv, rt1, 2, n / 2);
	fun_math_rotary_f32_cpu(rotx, cosv, sinv, rt2, 2, n / 2);
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
	fun_math_softmax_f32(x, 4);
	math_test_count_add(&tc, g_stub_softmax_called == 1);

	/* structured entries not provided by the stub fall back to the CPU */
	float a[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float b[4] = { 2.0f, 3.0f, 4.0f, 5.0f };
	float d = fun_math_dot_f32(a, b, 4);
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
	fun_math_unary(FUN_MATH_OP_EXP, (FunMathDtype)FUN_MATH_DTYPE_COUNT, &in,
				   &out);
	for (int i = 0; i < 4; i++)
		math_test_count_add(&tc, y[i] == 0.0f);

	return tc;
}

static TestCount test_new_binary_reduce_ops(void)
{
	TestCount tc = math_test_count_init();
	const int n = 32;
	float a[32];
	float b[32];
	float o[32];

	for (int i = 0; i < n; i++) {
		a[i] = (float)(i + 1);
		b[i] = (float)(i + 2);
	}

	fun_math_sub_f32(a, b, o, n);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] - b[i]);

	fun_math_div_f32(a, b, o, n);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] / b[i]);

	math_test_count_add(&tc, fun_math_min_f32(a, n) == 1.0f);
	math_test_count_add(&tc, fun_math_min_f32(a, 0) == 0.0f);

	float ref_mean = 0.0f;
	for (int i = 0; i < n; i++)
		ref_mean += a[i];
	ref_mean /= (float)n;
	math_test_count_add(&tc, fun_math_mean_f32(a, n) == ref_mean);
	math_test_count_add(&tc, fun_math_mean_f32(a, 0) == 0.0f);

	FunMathTensor ta = { a, (size_t)n, 1 };
	FunMathTensor tb = { b, (size_t)n, 1 };
	FunMathTensor to = { o, (size_t)n, 1 };

	fun_math_binary(FUN_MATH_OP_SUB, FUN_MATH_DTYPE_F32, &ta, &tb, &to);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] - b[i]);

	fun_math_binary(FUN_MATH_OP_DIV, FUN_MATH_DTYPE_F32, &ta, &tb, &to);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == a[i] / b[i]);

	float r = 0.0f;
	fun_math_reduce(FUN_MATH_OP_MIN, FUN_MATH_DTYPE_F32, &ta, &r);
	math_test_count_add(&tc, r == 1.0f);

	fun_math_reduce(FUN_MATH_OP_MEAN, FUN_MATH_DTYPE_F32, &ta, &r);
	math_test_count_add(&tc, _math_test_check_float(r, ref_mean, 1e-5f, 1e-4f));

	FunMathTensor tz = { a, 0, 1 };
	fun_math_reduce(FUN_MATH_OP_MIN, FUN_MATH_DTYPE_F32, &tz, &r);
	math_test_count_add(&tc, r == 0.0f);
	fun_math_reduce(FUN_MATH_OP_MEAN, FUN_MATH_DTYPE_F32, &tz, &r);
	math_test_count_add(&tc, r == 0.0f);

	return tc;
}

static TestCount test_f16_dispatch(void)
{
	TestCount tc = math_test_count_init();
	const int n = 16;
	uint16_t x[16];
	uint16_t a[16];
	uint16_t b[16];
	uint16_t o[16];

	math_test_count_add(&tc, fun_math_f32_to_fp16(0.0f) == 0x0000);
	math_test_count_add(&tc, fun_math_f32_to_fp16(-0.0f) == 0x8000);
	math_test_count_add(&tc, fun_math_f32_to_fp16(1.0f) == 0x3C00);
	math_test_count_add(&tc, fun_math_f32_to_fp16(-1.0f) == 0xBC00);
	math_test_count_add(&tc, fun_math_f32_to_fp16(65504.0f) == 0x7BFF);
	math_test_count_add(&tc,
						fun_math_f32_to_fp16(_math_test_make_inf()) == 0x7C00);
	math_test_count_add(&tc,
						fun_math_f32_to_fp16(_math_test_make_nan()) == 0x7E00);
	math_test_count_add(&tc, fun_math_f32_to_fp16(1.00048828125f) == 0x3C00);
	math_test_count_add(&tc, fun_math_f32_to_fp16(65520.0f) == 0x7C00);

	for (int i = 0; i < n; i++) {
		x[i] = fun_math_f32_to_fp16((float)(i - 8) * 0.25f);
		a[i] = fun_math_f32_to_fp16((float)(i + 1) * 0.5f);
		b[i] = fun_math_f32_to_fp16((float)(i + 2) * 0.25f);
	}

	FunMathTensor tx = { x, (size_t)n, 1 };
	FunMathTensor ta = { a, (size_t)n, 1 };
	FunMathTensor tb = { b, (size_t)n, 1 };
	FunMathTensor to = { o, (size_t)n, 1 };

	fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F16, &tx, &to);
	for (int i = 0; i < n; i++) {
		uint16_t want =
			fun_math_f32_to_fp16(fun_math_exp(fun_math_fp16_to_f32(x[i])));
		math_test_count_add(&tc, o[i] == want);
	}

	fun_math_binary(FUN_MATH_OP_ADD, FUN_MATH_DTYPE_F16, &ta, &tb, &to);
	for (int i = 0; i < n; i++) {
		uint16_t want = fun_math_f32_to_fp16(fun_math_fp16_to_f32(a[i]) +
											 fun_math_fp16_to_f32(b[i]));
		math_test_count_add(&tc, o[i] == want);
	}

	uint16_t sum_out = 0;
	fun_math_reduce(FUN_MATH_OP_SUM, FUN_MATH_DTYPE_F16, &ta, &sum_out);
	float ref_sum = 0.0f;
	for (int i = 0; i < n; i++)
		ref_sum += fun_math_fp16_to_f32(a[i]);
	math_test_count_add(&tc, sum_out == fun_math_f32_to_fp16(ref_sum));

	uint16_t max_out = 0;
	fun_math_reduce(FUN_MATH_OP_MAX, FUN_MATH_DTYPE_F16, &ta, &max_out);
	float ref_max = fun_math_fp16_to_f32(a[0]);
	for (int i = 1; i < n; i++) {
		float v = fun_math_fp16_to_f32(a[i]);
		if (v > ref_max)
			ref_max = v;
	}
	math_test_count_add(&tc, max_out == fun_math_f32_to_fp16(ref_max));

	return tc;
}

static TestCount test_f64_dispatch(void)
{
	TestCount tc = math_test_count_init();
	const int n = 16;
	double x[16];
	double a[16];
	double b[16];
	double o[16];

	for (int i = 0; i < n; i++) {
		x[i] = (double)((i + 1) * (i + 1));
		a[i] = (double)(i + 1);
		b[i] = (double)(i + 2) * 0.5;
	}

	FunMathTensor tx = { x, (size_t)n, 1 };
	FunMathTensor ta = { a, (size_t)n, 1 };
	FunMathTensor tb = { b, (size_t)n, 1 };
	FunMathTensor to = { o, (size_t)n, 1 };

	fun_math_unary(FUN_MATH_OP_SQRT, FUN_MATH_DTYPE_F64, &tx, &to);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == (double)fun_math_sqrt((float)x[i]));

	fun_math_binary(FUN_MATH_OP_SUB, FUN_MATH_DTYPE_F64, &ta, &tb, &to);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o[i] == (double)((float)a[i] - (float)b[i]));

	double mean_out = 0.0;
	fun_math_reduce(FUN_MATH_OP_MEAN, FUN_MATH_DTYPE_F64, &ta, &mean_out);
	float ref_mean = 0.0f;
	for (int i = 0; i < n; i++)
		ref_mean += (float)a[i];
	ref_mean /= (float)n;
	math_test_count_add(&tc, mean_out == (double)ref_mean);

	{
		double ex[4] = { -1.0, 0.0, 1.0, 2.0 };
		double eo[4];
		FunMathTensor tex = { ex, 4, 1 };
		FunMathTensor teo = { eo, 4, 1 };
		fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F64, &tex, &teo);
		for (int i = 0; i < 4; i++)
			math_test_count_add(&tc,
								eo[i] == (double)fun_math_exp((float)ex[i]));
		fun_math_unary(FUN_MATH_OP_TANH, FUN_MATH_DTYPE_F64, &tex, &teo);
		for (int i = 0; i < 4; i++)
			math_test_count_add(&tc,
								eo[i] == (double)fun_math_tanh((float)ex[i]));
	}

	return tc;
}

static TestCount test_new_structured_wrappers(void)
{
	TestCount tc = math_test_count_init();
	const size_t n = 8;
	const size_t n_rows = 4;
	const size_t row_len = 4;
	const size_t stride = 4;

	float gate[8];
	float up[8];
	float sw1[8];
	float sw2[8];
	float q[4];
	float rx[16];
	float rd1[4];
	float rd2[4];
	float wgt[4];
	float ws1[4];
	float ws2[4];
	uint8_t q8src[FUN_MATH_Q8_BLOCK_BYTES];
	float deq1[FUN_MATH_Q8_BLOCK_ELEMS];
	float deq2[FUN_MATH_Q8_BLOCK_ELEMS];
	float q8x[FUN_MATH_Q8_BLOCK_ELEMS];
	float mv1[1];
	float mv2[1];
	uint8_t mxw[FUN_MATH_MXFP4_BLOCK_BYTES] = { 0 };
	float mxx[FUN_MATH_MXFP4_BLOCK_ELEMS] = { 0 };
	float mx1[1];
	float mx2[1];

	uint32_t rng = _math_test_lcg_seed(23);
	for (size_t i = 0; i < n; i++) {
		gate[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
		up[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
	}
	for (size_t i = 0; i < row_len; i++)
		q[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
	for (size_t i = 0; i < n_rows * stride; i++)
		rx[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
	for (size_t i = 0; i < n_rows; i++)
		wgt[i] = _math_test_lcg_float(&rng, 0.5f, 1.5f);
	for (size_t i = 0; i < FUN_MATH_Q8_BLOCK_ELEMS; i++)
		q8x[i] = (float)i * 0.5f;

	fun_math_swiglu_f32(gate, up, sw1, n);
	fun_math_swiglu_f32_cpu(gate, up, sw2, n);
	for (size_t i = 0; i < n; i++)
		math_test_count_add(&tc, sw1[i] == sw2[i]);

	fun_math_rows_dot_f32(q, rx, rd1, n_rows, row_len, stride, 0.5f);
	fun_math_rows_dot_f32_cpu(q, rx, rd2, n_rows, row_len, stride, 0.5f);
	for (size_t i = 0; i < n_rows; i++)
		math_test_count_add(&tc, rd1[i] == rd2[i]);

	fun_math_weighted_sum_f32(wgt, rx, ws1, n_rows, row_len, stride);
	fun_math_weighted_sum_f32_cpu(wgt, rx, ws2, n_rows, row_len, stride);
	for (size_t i = 0; i < row_len; i++)
		math_test_count_add(&tc, ws1[i] == ws2[i]);

	q8src[0] = 0x00;
	q8src[1] = 0x3C;
	for (size_t j = 0; j < FUN_MATH_Q8_BLOCK_ELEMS; j++)
		q8src[2 + j] = (uint8_t)(int8_t)((int)j - 16);
	fun_math_q8_dequant_row_f32(q8src, deq1, FUN_MATH_Q8_BLOCK_ELEMS);
	fun_math_q8_dequant_row_f32_cpu(q8src, deq2, FUN_MATH_Q8_BLOCK_ELEMS);
	for (size_t j = 0; j < FUN_MATH_Q8_BLOCK_ELEMS; j++)
		math_test_count_add(&tc, deq1[j] == deq2[j]);

	fun_math_q8_matrix_vector_f32(q8src, q8x, mv1, 1, FUN_MATH_Q8_BLOCK_ELEMS);
	fun_math_q8_matrix_vector_f32_cpu(q8src, q8x, mv2, 1,
									  FUN_MATH_Q8_BLOCK_ELEMS);
	math_test_count_add(&tc, mv1[0] == mv2[0]);

	fun_math_matrix_vector_mxfp4_f32(mxw, mxx, NULL, mx1, 1,
									 FUN_MATH_MXFP4_BLOCK_ELEMS);
	fun_math_matrix_vector_mxfp4_f32_cpu(mxw, mxx, NULL, mx2, 1,
										 FUN_MATH_MXFP4_BLOCK_ELEMS);
	math_test_count_add(&tc, mx1[0] == mx2[0]);

	fun_math_matrix_vector_mxfp4_f32_strided(mxw, mxx, NULL, mx1, 1,
											 FUN_MATH_MXFP4_BLOCK_ELEMS,
											 FUN_MATH_MXFP4_BLOCK_BYTES);
	fun_math_matrix_vector_mxfp4_f32_strided_cpu(mxw, mxx, NULL, mx2, 1,
												 FUN_MATH_MXFP4_BLOCK_ELEMS,
												 FUN_MATH_MXFP4_BLOCK_BYTES);
	math_test_count_add(&tc, mx1[0] == mx2[0]);

	g_stub_swiglu_called = 0;
	g_stub_rows_dot_called = 0;
	fun_math_backend_set(&g_stub_structured_backend);
	fun_math_swiglu_f32(gate, up, sw1, n);
	math_test_count_add(&tc, g_stub_swiglu_called == 1);
	fun_math_rows_dot_f32(q, rx, rd1, n_rows, row_len, stride, 0.5f);
	math_test_count_add(&tc, g_stub_rows_dot_called == 1);
	fun_math_backend_set(NULL);

	return tc;
}

static TestCount test_unary_fastpath(void)
{
	TestCount tc = math_test_count_init();
	const int n = 64;
	float x[64];
	float o1[64];
	float o2[64];

	uint32_t rng = _math_test_lcg_seed(29);
	for (int i = 0; i < n; i++)
		x[i] = _math_test_lcg_float(&rng, -4.0f, 4.0f);

	FunMathTensor in = { x, (size_t)n, 1 };
	FunMathTensor out = { o1, (size_t)n, 1 };
	fun_math_unary(FUN_MATH_OP_EXP, FUN_MATH_DTYPE_F32, &in, &out);
	fun_math_exp_f32(x, o2, n);
	for (int i = 0; i < n; i++)
		math_test_count_add(&tc, o1[i] == o2[i]);

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

	fun_console_write("    new binary/reduce ops: ");
	tc = test_new_binary_reduce_ops();
	math_test_count_merge(&total, tc);

	fun_console_write("    f16 dispatch: ");
	tc = test_f16_dispatch();
	math_test_count_merge(&total, tc);

	fun_console_write("    f64 dispatch: ");
	tc = test_f64_dispatch();
	math_test_count_merge(&total, tc);

	fun_console_write("    new structured wrappers: ");
	tc = test_new_structured_wrappers();
	math_test_count_merge(&total, tc);

	fun_console_write("    unary fast-path: ");
	tc = test_unary_fastpath();
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
