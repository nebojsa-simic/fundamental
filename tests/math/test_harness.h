#ifndef MATH_TEST_HARNESS_H
#define MATH_TEST_HARNESS_H

#include <stddef.h>
#include <stdint.h>

#define MATH_TEST_PI 3.14159265358979323846f

typedef struct {
	int passed;
	int failed;
} TestCount;

static inline TestCount math_test_count_init(void)
{
	TestCount tc = { 0, 0 };
	return tc;
}

static inline void math_test_count_add(TestCount *tc, int passed)
{
	if (passed) {
		tc->passed++;
	} else {
		tc->failed++;
	}
}

static inline void math_test_count_merge(TestCount *dst, TestCount src)
{
	dst->passed += src.passed;
	dst->failed += src.failed;
}

static inline int math_test_count_ok(TestCount tc)
{
	return tc.failed == 0;
}

static inline uint32_t _math_test_lcg_seed(uint32_t seed)
{
	return seed ? seed : 1;
}

static inline uint32_t _math_test_lcg_next(uint32_t *state)
{
	*state = *state * 1103515245 + 12345;
	return *state;
}

static inline float _math_test_lcg_float(uint32_t *state, float lo, float hi)
{
	uint32_t r = _math_test_lcg_next(state);
	return lo + ((float)(r & 0x7FFFFFFFu) / (float)0x7FFFFFFFu) * (hi - lo);
}

static inline int _math_test_float_is_nan(float x)
{
	uint32_t u = *(uint32_t *)&x;
	return (u & 0x7F800000u) == 0x7F800000u && (u & 0x007FFFFFu) != 0;
}

static inline int _math_test_float_is_inf(float x)
{
	uint32_t u = *(uint32_t *)&x;
	return (u & 0x7FFFFFFFu) == 0x7F800000u;
}

static inline int _math_test_float_is_neg(float x)
{
	return (*(uint32_t *)&x & 0x80000000u) != 0;
}

static inline float _math_test_make_nan(void)
{
	uint32_t u = 0x7FC00000u;
	return *(float *)&u;
}

static inline float _math_test_make_inf(void)
{
	uint32_t u = 0x7F800000u;
	return *(float *)&u;
}

static inline float _math_test_make_neg_inf(void)
{
	uint32_t u = 0xFF800000u;
	return *(float *)&u;
}

static inline float _math_test_make_neg_zero(void)
{
	uint32_t u = 0x80000000u;
	return *(float *)&u;
}

static inline float _math_test_absf(float x)
{
	uint32_t u = *(uint32_t *)&x;
	u &= 0x7FFFFFFFu;
	return *(float *)&u;
}

static inline int _math_test_check_float(float got, float expected,
										 float abs_tol, float rel_tol)
{
	float diff = got - expected;
	if (diff < 0.0f)
		diff = -diff;
	float abs_expected = expected;
	if (abs_expected < 0.0f)
		abs_expected = -abs_expected;
	if (abs_expected < 1e-10f) {
		return diff <= abs_tol;
	}
	return diff <= abs_tol || diff / abs_expected <= rel_tol;
}

static inline int _math_test_check_nan(float x)
{
	return _math_test_float_is_nan(x);
}

static inline int _math_test_check_same_sign(float a, float b)
{
	return _math_test_float_is_neg(a) == _math_test_float_is_neg(b);
}

float fun_math_sqrt(float x);
float fun_math_exp(float x);
float fun_math_log(float x);
float fun_math_sin(float x);
float fun_math_cos(float x);
float fun_math_tanh(float x);
float fun_math_sigmoid(float x);
float fun_math_silu(float x);

void fun_math_silu_f32(const float *x, float *out, size_t n);
void fun_math_rms_norm_f32(const float *x, const float *weight, float *out,
						   size_t n, float eps);
void fun_math_swiglu_f32(const float *gate, const float *up, float *out,
						 size_t n);
void fun_math_softmax_f32(float *x, size_t n);

void fun_math_matrix_vector_f32(const float *w, const float *x,
								const float *bias, float *out, size_t rows,
								size_t cols);
float fun_math_dot_f32(const float *a, const float *b, size_t n);

void fun_math_exp_f32(const float *x, float *out, size_t n);
void fun_math_log_f32(const float *x, float *out, size_t n);
void fun_math_sin_f32(const float *x, float *out, size_t n);
void fun_math_cos_f32(const float *x, float *out, size_t n);

void fun_math_rotary_f32(const float *x, const float *cosv, const float *sinv,
						 float *out, size_t n_heads, size_t half);

void _math_bench_noop(const float *x, float *out, size_t n);

#endif
