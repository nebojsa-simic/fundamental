#include "fundamental/math/math.h"

#include <stddef.h>

void _silu_f32_avx2(const float *x, float *out, size_t n);
void _swiglu_f32_avx2(const float *gate, const float *up, float *out, size_t n);
void _rms_norm_f32_avx2(const float *x, const float *weight, float *out,
						size_t n, float eps);
void _softmax_f32_avx2(float *x, size_t n);

static void _silu_f32_scalar(const float *x, float *out, size_t n)
{
	for (size_t i = 0; i < n; i++)
		out[i] = fun_math_silu(x[i]);
}

static void _swiglu_f32_scalar(const float *gate, const float *up, float *out,
							   size_t n)
{
	for (size_t i = 0; i < n; i++) {
		float s = fun_math_sigmoid(gate[i]);
		out[i] = gate[i] * s * up[i];
	}
}

static void _rms_norm_f32_scalar(const float *x, const float *weight,
								 float *out, size_t n, float eps)
{
	float ss = 0.0f;
	for (size_t i = 0; i < n; i++)
		ss += x[i] * x[i];
	float scale = 1.0f / fun_math_sqrt(ss / (float)n + eps);
	for (size_t i = 0; i < n; i++)
		out[i] = x[i] * scale * weight[i];
}

static void _softmax_f32_scalar(float *x, size_t n)
{
	float m = x[0];
	for (size_t i = 1; i < n; i++)
		if (x[i] > m)
			m = x[i];
	float sum = 0.0f;
	for (size_t i = 0; i < n; i++) {
		x[i] = fun_math_exp(x[i] - m);
		sum += x[i];
	}
	for (size_t i = 0; i < n; i++)
		x[i] /= sum;
}

typedef void (*SiluF32Fn)(const float *, float *, size_t);
typedef void (*SwigluF32Fn)(const float *, const float *, float *, size_t);
typedef void (*RmsNormF32Fn)(const float *, const float *, float *, size_t,
							 float);
typedef void (*SoftmaxF32Fn)(float *, size_t);

static SiluF32Fn _silu_f32_impl = _silu_f32_scalar;
static SwigluF32Fn _swiglu_f32_impl = _swiglu_f32_scalar;
static RmsNormF32Fn _rms_norm_f32_impl = _rms_norm_f32_scalar;
static SoftmaxF32Fn _softmax_f32_impl = _softmax_f32_scalar;

void _math_dispatch_init(void)
{
	if (fun_math_has_avx2()) {
		_silu_f32_impl = _silu_f32_avx2;
		_swiglu_f32_impl = _swiglu_f32_avx2;
		_rms_norm_f32_impl = _rms_norm_f32_avx2;
		_softmax_f32_impl = _softmax_f32_avx2;
	}
}

void fun_math_silu_f32(const float *x, float *out, size_t n)
{
	_silu_f32_impl(x, out, n);
}

void fun_math_swiglu_f32(const float *gate, const float *up, float *out,
						 size_t n)
{
	_swiglu_f32_impl(gate, up, out, n);
}

void fun_math_rms_norm_f32(const float *x, const float *weight, float *out,
						   size_t n, float eps)
{
	_rms_norm_f32_impl(x, weight, out, n, eps);
}

void fun_math_softmax_f32(float *x, size_t n)
{
	_softmax_f32_impl(x, n);
}
