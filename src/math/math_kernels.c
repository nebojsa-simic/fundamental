#include "fundamental/math/math_kernels.h"

static const FunMathBackend *g_backend = NULL;

static void _cpu_unary(FunMathOp op, FunMathDtype dtype,
					   const FunMathTensor *in, FunMathTensor *out)
{
	if (dtype != FUN_MATH_DTYPE_F32)
		return;
	const float *x = (const float *)in->data;
	float *y = (float *)out->data;
	for (size_t i = 0; i < in->n; i++) {
		float v = x[i * in->stride];
		float r;
		switch (op) {
		case FUN_MATH_OP_SQRT:
			r = fun_math_sqrt(v);
			break;
		case FUN_MATH_OP_EXP:
			r = fun_math_exp(v);
			break;
		case FUN_MATH_OP_LOG:
			r = fun_math_log(v);
			break;
		case FUN_MATH_OP_SIN:
			r = fun_math_sin(v);
			break;
		case FUN_MATH_OP_COS:
			r = fun_math_cos(v);
			break;
		case FUN_MATH_OP_TANH:
			r = fun_math_tanh(v);
			break;
		case FUN_MATH_OP_SIGMOID:
			r = fun_math_sigmoid(v);
			break;
		case FUN_MATH_OP_SILU:
			r = fun_math_silu(v);
			break;
		default:
			r = v;
			break;
		}
		y[i * out->stride] = r;
	}
}

static void _cpu_binary(FunMathOp op, FunMathDtype dtype,
						const FunMathTensor *a, const FunMathTensor *b,
						FunMathTensor *out)
{
	if (dtype != FUN_MATH_DTYPE_F32)
		return;
	const float *pa = (const float *)a->data;
	const float *pb = (const float *)b->data;
	float *po = (float *)out->data;
	if (a->stride == 1 && b->stride == 1 && out->stride == 1) {
		if (op == FUN_MATH_OP_ADD)
			fun_math_add_f32(pa, pb, po, a->n);
		else if (op == FUN_MATH_OP_MUL)
			fun_math_mul_f32(pa, pb, po, a->n);
		return;
	}
	for (size_t i = 0; i < a->n; i++) {
		float x = pa[i * a->stride];
		float y = pb[i * b->stride];
		if (op == FUN_MATH_OP_ADD)
			po[i * out->stride] = x + y;
		else if (op == FUN_MATH_OP_MUL)
			po[i * out->stride] = x * y;
	}
}

static void _cpu_reduce(FunMathOp op, FunMathDtype dtype,
						const FunMathTensor *in, void *out)
{
	if (dtype != FUN_MATH_DTYPE_F32)
		return;
	const float *x = (const float *)in->data;
	float *o = (float *)out;
	if (in->stride == 1) {
		if (op == FUN_MATH_OP_SUM)
			*o = fun_math_sum_f32(x, in->n);
		else
			*o = fun_math_max_f32(x, in->n);
		return;
	}
	if (op == FUN_MATH_OP_SUM) {
		float s = 0.0f;
		for (size_t i = 0; i < in->n; i++)
			s += x[i * in->stride];
		*o = s;
	} else {
		float m = 0.0f;
		if (in->n > 0) {
			m = x[0];
			for (size_t i = 1; i < in->n; i++)
				if (x[i * in->stride] > m)
					m = x[i * in->stride];
		}
		*o = m;
	}
}

static const FunMathBackend g_cpu_backend = {
	.unary = _cpu_unary,
	.binary = _cpu_binary,
	.reduce = _cpu_reduce,
	.matrix_vector_f32 = fun_math_matrix_vector_f32,
	.dot_f32 = fun_math_dot_f32,
	.rms_norm_f32 = fun_math_rms_norm_f32,
	.softmax_f32 = fun_math_softmax_f32,
	.rotary_f32 = fun_math_rotary_f32,
};

void fun_math_backend_set(const FunMathBackend *backend)
{
	g_backend = backend;
}

const FunMathBackend *fun_math_backend_get(void)
{
	return g_backend ? g_backend : &g_cpu_backend;
}

void fun_math_unary(FunMathOp op, FunMathDtype dtype, const FunMathTensor *in,
					FunMathTensor *out)
{
	const FunMathBackend *b = fun_math_backend_get();
	if (b->unary)
		b->unary(op, dtype, in, out);
	else
		g_cpu_backend.unary(op, dtype, in, out);
}

void fun_math_binary(FunMathOp op, FunMathDtype dtype, const FunMathTensor *a,
					 const FunMathTensor *b, FunMathTensor *out)
{
	const FunMathBackend *bk = fun_math_backend_get();
	if (bk->binary)
		bk->binary(op, dtype, a, b, out);
	else
		g_cpu_backend.binary(op, dtype, a, b, out);
}

void fun_math_reduce(FunMathOp op, FunMathDtype dtype, const FunMathTensor *in,
					 void *out)
{
	const FunMathBackend *b = fun_math_backend_get();
	if (b->reduce)
		b->reduce(op, dtype, in, out);
	else
		g_cpu_backend.reduce(op, dtype, in, out);
}

void fun_math_matrix_vector_f32_dispatch(const float *w, const float *x,
										 const float *bias, float *out,
										 size_t rows, size_t cols)
{
	const FunMathBackend *b = fun_math_backend_get();
	if (b->matrix_vector_f32)
		b->matrix_vector_f32(w, x, bias, out, rows, cols);
	else
		fun_math_matrix_vector_f32(w, x, bias, out, rows, cols);
}

float fun_math_dot_f32_dispatch(const float *a, const float *b, size_t n)
{
	const FunMathBackend *bk = fun_math_backend_get();
	if (bk->dot_f32)
		return bk->dot_f32(a, b, n);
	return fun_math_dot_f32(a, b, n);
}

void fun_math_rms_norm_f32_dispatch(const float *x, const float *weight,
									float *out, size_t n, float eps)
{
	const FunMathBackend *b = fun_math_backend_get();
	if (b->rms_norm_f32)
		b->rms_norm_f32(x, weight, out, n, eps);
	else
		fun_math_rms_norm_f32(x, weight, out, n, eps);
}

void fun_math_softmax_f32_dispatch(float *x, size_t n)
{
	const FunMathBackend *b = fun_math_backend_get();
	if (b->softmax_f32)
		b->softmax_f32(x, n);
	else
		fun_math_softmax_f32(x, n);
}

void fun_math_rotary_f32_dispatch(const float *x, const float *cosv,
								  const float *sinv, float *out, size_t n_heads,
								  size_t half)
{
	const FunMathBackend *b = fun_math_backend_get();
	if (b->rotary_f32)
		b->rotary_f32(x, cosv, sinv, out, n_heads, half);
	else
		fun_math_rotary_f32(x, cosv, sinv, out, n_heads, half);
}
