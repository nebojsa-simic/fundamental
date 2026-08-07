#include "fundamental/math/math.h"

#include <stdint.h>

#define L2E 1.4426950408889634f
#define LN2 0.6931471805599453f
#define PI 3.14159265358979323846f
#define TWOPI 6.28318530717958647692f
#define HALFPI 1.57079632679489661923f

static float _nan(void)
{
	uint32_t u = 0x7FC00000u;
	return *(float *)&u;
}

static float _inf(void)
{
	uint32_t u = 0x7F800000u;
	return *(float *)&u;
}

static float _neginf(void)
{
	uint32_t u = 0xFF800000u;
	return *(float *)&u;
}

static int _is_nan(float x)
{
	uint32_t u = *(uint32_t *)&x;
	return (u & 0x7F800000u) == 0x7F800000u && (u & 0x007FFFFFu) != 0;
}

static int _is_inf(float x)
{
	uint32_t u = *(uint32_t *)&x;
	return (u & 0x7FFFFFFFu) == 0x7F800000u;
}

float fun_math_sqrt(float x)
{
	if (_is_nan(x))
		return x;
	if (x < 0.0f) {
		uint32_t u = *(uint32_t *)&x;
		if (u == 0x80000000u)
			return x;
		return _nan();
	}
	if (x == 0.0f)
		return x;
	if (_is_inf(x))
		return x;

	float half_x = 0.5f * x;
	int i = *(int *)&x;
	i = 0x5f3759df - (i >> 1);
	float y = *(float *)&i;
	y = y * (1.5f - half_x * y * y);
	y = y * (1.5f - half_x * y * y);
	return x * y;
}

float fun_math_exp(float x)
{
	if (_is_nan(x))
		return x;
	if (x >= 88.7228f)
		return _inf();
	if (x <= -87.3365f)
		return 0.0f;
	if (_is_inf(x)) {
		if (x > 0.0f)
			return _inf();
		return 0.0f;
	}

	float kf = x * L2E + 0.5f;
	int k = (int)kf;
	if (kf < k)
		k--;
	float r = x - (float)k * LN2;

	float c0 = 1.0f;
	float c1 = 0.5f;
	float c2 = 0.10714285714285714f;
	float c3 = 0.011904761904761904f;
	float c4 = 0.0005952380952380952f;

	float r2 = r * r;
	float num = c0 + c1 * r + c2 * r2 + c3 * r * r2 + c4 * r2 * r2;
	float den = c0 - c1 * r + c2 * r2 - c3 * r * r2 + c4 * r2 * r2;
	float er = num / den;

	uint32_t bits = (uint32_t)(k + 127) << 23;
	return er * (*(float *)&bits);
}

float fun_math_log(float x)
{
	if (_is_nan(x))
		return x;
	uint32_t u = *(uint32_t *)&x;
	if (u == 0x80000000u || u == 0x00000000u)
		return _neginf();
	if (x < 0.0f)
		return _nan();
	if (_is_inf(x))
		return x;

	uint32_t bits = *(uint32_t *)&x;
	int exp_field = (int)((bits >> 23) & 0xFFu);
	int n = exp_field - 127;
	uint32_t mantissa_bits = (bits & 0x7FFFFFu) | 0x3F800000u;
	float m = *(float *)&mantissa_bits;

	float t = (m - 1.0f) / (m + 1.0f);
	float t2 = t * t;
	float lg_m =
		2.0f * t *
		(1.0f + t2 * (0.333333333f +
					  t2 * (0.2f + t2 * (0.142857143f + t2 * 0.111111111f))));

	return (float)n * LN2 + lg_m;
}

float fun_math_sin(float x)
{
	if (_is_nan(x))
		return x;
	float nf = x / TWOPI + 0.5f;
	int n = (int)nf;
	if (nf < (float)n)
		n--;
	x = x - (float)n * TWOPI;
	int negate = 0;
	if (x < 0.0f) {
		x = -x;
		negate = 1;
	}
	if (x > PI) {
		x = x - TWOPI;
		if (x < 0.0f)
			x = -x;
	}
	if (x > HALFPI)
		x = PI - x;
	if (x > HALFPI - 1e-7f)
		return negate ? -1.0f : 1.0f;
	float x2 = x * x;
	float x3 = x2 * x;
	float x5 = x3 * x2;
	float x7 = x5 * x2;
	float x9 = x7 * x2;
	float result = x - x3 * 0.1666666667f + x5 * 0.008333333333f -
				   x7 * 0.0001984126984f + x9 * 0.000002755731922f;
	return negate ? -result : result;
}

float fun_math_cos(float x)
{
	return fun_math_sin(x + HALFPI);
}

float fun_math_tanh(float x)
{
	if (_is_nan(x))
		return x;
	if (x >= 10.0f)
		return 1.0f;
	if (x <= -10.0f)
		return -1.0f;
	float e2x = fun_math_exp(2.0f * x);
	return (e2x - 1.0f) / (e2x + 1.0f);
}

float fun_math_sigmoid(float x)
{
	if (_is_nan(x))
		return x;
	if (x >= 20.0f)
		return 1.0f;
	if (x <= -20.0f)
		return 0.0f;
	if (x >= 0.0f)
		return 1.0f / (1.0f + fun_math_exp(-x));
	float ex = fun_math_exp(x);
	return ex / (1.0f + ex);
}

float fun_math_silu(float x)
{
	if (_is_nan(x))
		return x;
	if (x >= 20.0f)
		return x;
	if (x <= -20.0f)
		return 0.0f;
	return x * fun_math_sigmoid(x);
}

void fun_math_silu_f32(const float *x, float *out, size_t n)
{
	(void)x;
	(void)out;
	(void)n;
}

void fun_math_rms_norm_f32(const float *x, const float *weight, float *out,
						   size_t n, float eps)
{
	(void)x;
	(void)weight;
	(void)out;
	(void)n;
	(void)eps;
}

void fun_math_swiglu_f32(const float *gate, const float *up, float *out,
						 size_t n)
{
	(void)gate;
	(void)up;
	(void)out;
	(void)n;
}

void fun_math_softmax_f32(float *x, size_t n)
{
	(void)x;
	(void)n;
}

void _math_bench_noop(const float *x, float *out, size_t n)
{
	for (size_t i = 0; i < n; i++)
		out[i] = x[i];
}
