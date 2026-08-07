#include <immintrin.h>

#define L2E 1.4426950408889634f
#define LN2 0.6931471805599453f

static __m256 _mm256_exp_ps(__m256 x)
{
	__m256 l2e = _mm256_set1_ps(L2E);
	__m256 half = _mm256_set1_ps(0.5f);
	__m256 kf = _mm256_add_ps(_mm256_mul_ps(x, l2e), half);
	__m256i k = _mm256_cvtps_epi32(kf);

	__m256 ln2 = _mm256_set1_ps(LN2);
	__m256 r = _mm256_fnmadd_ps(_mm256_cvtepi32_ps(k), ln2, x);

	__m256 c0 = _mm256_set1_ps(1.0f);
	__m256 c1 = _mm256_set1_ps(0.5f);
	__m256 c2 = _mm256_set1_ps(0.10714285714285714f);
	__m256 c3 = _mm256_set1_ps(0.011904761904761904f);
	__m256 c4 = _mm256_set1_ps(0.0005952380952380952f);

	__m256 r2 = _mm256_mul_ps(r, r);

	__m256 num = _mm256_fmadd_ps(c1, r, c0);
	num = _mm256_fmadd_ps(c2, r2, num);
	num = _mm256_fmadd_ps(c3, _mm256_mul_ps(r, r2), num);
	num = _mm256_fmadd_ps(c4, _mm256_mul_ps(r2, r2), num);

	__m256 den = _mm256_fnmadd_ps(c1, r, c0);
	den = _mm256_fmadd_ps(c2, r2, den);
	den = _mm256_fnmadd_ps(c3, _mm256_mul_ps(r, r2), den);
	den = _mm256_fmadd_ps(c4, _mm256_mul_ps(r2, r2), den);

	__m256 er = _mm256_div_ps(num, den);

	__m256i exp_bits =
		_mm256_slli_epi32(_mm256_add_epi32(k, _mm256_set1_epi32(127)), 23);
	__m256 pow2 = _mm256_castsi256_ps(exp_bits);

	return _mm256_mul_ps(er, pow2);
}

static __m256 _mm256_sigmoid_ps(__m256 x)
{
	__m256 one = _mm256_set1_ps(1.0f);
	__m256 zero = _mm256_set1_ps(0.0f);
	__m256 pos_hi = _mm256_cmp_ps(x, _mm256_set1_ps(20.0f), _CMP_GE_OQ);
	__m256 neg_hi = _mm256_cmp_ps(x, _mm256_set1_ps(-20.0f), _CMP_LE_OQ);
	__m256 pos = _mm256_cmp_ps(x, zero, _CMP_GE_OQ);
	__m256 neg = _mm256_cmp_ps(x, zero, _CMP_LT_OQ);

	__m256 nx = _mm256_sub_ps(zero, x);
	__m256 px = x;

	__m256 ex_neg = _mm256_div_ps(one, _mm256_add_ps(one, _mm256_exp_ps(nx)));
	__m256 denom = _mm256_add_ps(one, _mm256_exp_ps(px));
	__m256 ex_pos = _mm256_div_ps(one, denom);

	__m256 result = _mm256_blendv_ps(ex_pos, ex_neg, neg);

	result = _mm256_blendv_ps(result, one, pos_hi);
	result = _mm256_blendv_ps(result, zero, neg_hi);

	return result;
}

static __m256 _mm256_silu_ps(__m256 x)
{
	return _mm256_mul_ps(x, _mm256_sigmoid_ps(x));
}

static float _mm256_hsum_ps(__m256 v)
{
	__m128 hi = _mm256_extractf128_ps(v, 1);
	__m128 lo = _mm256_castps256_ps128(v);
	__m128 sum = _mm_add_ps(lo, hi);
	sum = _mm_hadd_ps(sum, sum);
	sum = _mm_hadd_ps(sum, sum);
	return _mm_cvtss_f32(sum);
}

static float _mm256_hmax_ps(__m256 v)
{
	__m128 hi = _mm256_extractf128_ps(v, 1);
	__m128 lo = _mm256_castps256_ps128(v);
	__m128 mx = _mm_max_ps(lo, hi);
	mx = _mm_max_ps(mx, _mm_permute_ps(mx, 0x0E));
	mx = _mm_max_ps(mx, _mm_permute_ps(mx, 0x01));
	return _mm_cvtss_f32(mx);
}

void _silu_f32_avx2(const float *x, float *out, size_t n)
{
	for (size_t i = 0; i < n; i += 8) {
		__m256 v = _mm256_load_ps(x + i);
		__m256 r = _mm256_silu_ps(v);
		_mm256_store_ps(out + i, r);
	}
}

void _swiglu_f32_avx2(const float *gate, const float *up, float *out, size_t n)
{
	for (size_t i = 0; i < n; i += 8) {
		__m256 g = _mm256_load_ps(gate + i);
		__m256 u = _mm256_load_ps(up + i);
		__m256 sg = _mm256_silu_ps(g);
		_mm256_store_ps(out + i, _mm256_mul_ps(sg, u));
	}
}

void _rms_norm_f32_avx2(const float *x, const float *weight, float *out,
						size_t n, float eps)
{
	__m256 ss = _mm256_setzero_ps();
	for (size_t i = 0; i < n; i += 8) {
		__m256 v = _mm256_load_ps(x + i);
		ss = _mm256_fmadd_ps(v, v, ss);
	}
	float ss_val = _mm256_hsum_ps(ss);
	float scale = 1.0f / __builtin_sqrtf(ss_val / (float)n + eps);

	__m256 s256 = _mm256_set1_ps(scale);
	for (size_t i = 0; i < n; i += 8) {
		__m256 v = _mm256_load_ps(x + i);
		__m256 w = _mm256_load_ps(weight + i);
		_mm256_store_ps(out + i, _mm256_mul_ps(_mm256_mul_ps(v, s256), w));
	}
}

void _softmax_f32_avx2(float *x, size_t n)
{
	__m256 mx = _mm256_load_ps(x);
	for (size_t i = 8; i < n; i += 8) {
		__m256 v = _mm256_load_ps(x + i);
		mx = _mm256_max_ps(mx, v);
	}
	float m = _mm256_hmax_ps(mx);

	__m256 sum = _mm256_setzero_ps();
	__m256 m256 = _mm256_set1_ps(m);
	for (size_t i = 0; i < n; i += 8) {
		__m256 v = _mm256_load_ps(x + i);
		__m256 d = _mm256_sub_ps(v, m256);
		__m256 e = _mm256_exp_ps(d);
		_mm256_store_ps(x + i, e);
		sum = _mm256_add_ps(sum, e);
	}
	float s = _mm256_hsum_ps(sum);

	__m256 s256 = _mm256_set1_ps(s);
	for (size_t i = 0; i < n; i += 8) {
		__m256 v = _mm256_load_ps(x + i);
		_mm256_store_ps(x + i, _mm256_div_ps(v, s256));
	}
}
