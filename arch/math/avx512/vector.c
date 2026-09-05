#include "fundamental/math/math.h"
#include <immintrin.h>

/* MXFP4 E2M1 kvalue table multiplied by two: the nibble lookup is done with
 * vpshufb over this table, so the scale byte (a power of two) is applied one
 * exponent lower: scale_f = bit_cast<float>((scale - 1) << 23). A scale byte
 * of zero decodes to 0.0f and skips the block entirely; a scale byte of 1
 * decodes to 2^-127, which the shifted exponent cannot represent (it would
 * be 0.0f), so it is handled with an explicit subnormal bit pattern. */
static const int8_t mxfp4_kvalues_x2[16] = { 0, 1,	2,	3,	4,	6,	8,	12,
											 0, -1, -2, -3, -4, -6, -8, -12 };

static inline float __attribute__((always_inline)) _mm256_hsum_ps(__m256 v)
{
	__m128 hi = _mm256_extractf128_ps(v, 1);
	__m128 lo = _mm256_castps256_ps128(v);
	__m128 sum = _mm_add_ps(lo, hi);
	sum = _mm_hadd_ps(sum, sum);
	sum = _mm_hadd_ps(sum, sum);
	return _mm_cvtss_f32(sum);
}

/* Decode one 16-byte MXFP4 block payload into four __m256 float kvalue
 * vectors covering the 32 elements, in the same order as the AVX2
 * implementation (low nibbles at elements 0..15, high nibbles at 16..31). */
static inline void __attribute__((always_inline))
_mxfp4_dequant_block(const uint8_t *nibbles, const __m128i tab,
					 const __m128i mask04, __m256 *k0, __m256 *k1, __m256 *k2,
					 __m256 *k3)
{
	__m128i n = _mm_loadu_si128((const __m128i *)nibbles);
	__m128i lo_i = _mm_shuffle_epi8(tab, _mm_and_si128(n, mask04));
	__m128i hi_i =
		_mm_shuffle_epi8(tab, _mm_and_si128(_mm_srli_epi16(n, 4), mask04));
	*k0 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(lo_i));
	*k1 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(lo_i, 8)));
	*k2 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(hi_i));
	*k3 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(hi_i, 8)));
}

/* Decode one block and dot it against 32 floats, accumulating unscaled. */
static inline __m256 __attribute__((always_inline))
_mxfp4_block_dot(const uint8_t *nibbles, const float *xv, const __m128i tab,
				 const __m128i mask04)
{
	__m256 k0, k1, k2, k3;
	_mxfp4_dequant_block(nibbles, tab, mask04, &k0, &k1, &k2, &k3);
	__m256 bs = _mm256_setzero_ps();
	bs = _mm256_fmadd_ps(k0, _mm256_loadu_ps(xv), bs);
	bs = _mm256_fmadd_ps(k1, _mm256_loadu_ps(xv + 8), bs);
	bs = _mm256_fmadd_ps(k2, _mm256_loadu_ps(xv + 16), bs);
	bs = _mm256_fmadd_ps(k3, _mm256_loadu_ps(xv + 24), bs);
	return bs;
}

static inline __m256 __attribute__((always_inline))
_mxfp4_scale_broadcast(uint8_t scale, __m256 sv_scale1)
{
	if (scale == 1)
		return sv_scale1;
	return _mm256_castsi256_ps(
		_mm256_set1_epi32(((uint32_t)(scale - 1)) << 23));
}

void fun_math_matrix_vector_mxfp4_f32(const uint8_t *w, const float *x,
									  const float *bias, float *out,
									  size_t rows, size_t cols)
{
	fun_math_matrix_vector_mxfp4_f32_strided(w, x, bias, out, rows, cols, 0);
}

void fun_math_matrix_vector_mxfp4_f32_strided(const uint8_t *w, const float *x,
											  const float *bias, float *out,
											  size_t rows, size_t cols,
											  size_t row_stride_bytes)
{
	const size_t blocks = cols / FUN_MATH_MXFP4_BLOCK_ELEMS;
	const size_t row_bytes = blocks * FUN_MATH_MXFP4_BLOCK_BYTES;
	const size_t stride = row_stride_bytes > 0 ? row_stride_bytes : row_bytes;

	__m128i tab = _mm_loadu_si128((const __m128i *)mxfp4_kvalues_x2);
	__m128i mask04 = _mm_set1_epi8(0x0F);
	__m256 sv_scale1 = _mm256_castsi256_ps(_mm256_set1_epi32(0x00400000u));

	for (size_t r = 0; r < rows; r++) {
		const uint8_t *bp = w + r * stride;
		__m512 acc = _mm512_setzero_ps();
		size_t b = 0;

		/* Process two blocks (64 elements) per pass: decode each block with
		 * the exact AVX2 128-bit path, accumulate both into one ZMM. */
		for (; b + 1 < blocks; b += 2, bp += 2 * FUN_MATH_MXFP4_BLOCK_BYTES) {
			uint8_t s0 = bp[0];
			uint8_t s1 = bp[FUN_MATH_MXFP4_BLOCK_BYTES];
			__m256 lo = _mm256_setzero_ps();
			__m256 hi = _mm256_setzero_ps();

			if (s0 != 0) {
				__m256 bs = _mxfp4_block_dot(
					bp + 1, x + b * FUN_MATH_MXFP4_BLOCK_ELEMS, tab, mask04);
				lo = _mm256_mul_ps(bs, _mxfp4_scale_broadcast(s0, sv_scale1));
			}
			if (s1 != 0) {
				__m256 bs = _mxfp4_block_dot(
					bp + FUN_MATH_MXFP4_BLOCK_BYTES + 1,
					x + (b + 1) * FUN_MATH_MXFP4_BLOCK_ELEMS, tab, mask04);
				hi = _mm256_mul_ps(bs, _mxfp4_scale_broadcast(s1, sv_scale1));
			}

			__m512 pair = _mm512_insertf32x8(_mm512_castps256_ps512(lo), hi, 1);
			acc = _mm512_add_ps(acc, pair);
		}

		/* Tail: a single odd block. */
		if (b < blocks) {
			uint8_t s0 = bp[0];
			if (s0 != 0) {
				__m256 bs = _mxfp4_block_dot(
					bp + 1, x + b * FUN_MATH_MXFP4_BLOCK_ELEMS, tab, mask04);
				bs = _mm256_mul_ps(bs, _mxfp4_scale_broadcast(s0, sv_scale1));
				acc = _mm512_add_ps(acc, _mm512_castps256_ps512(bs));
			}
		}

		__m256 lo = _mm512_castps512_ps256(acc);
		__m256 hi = _mm512_extractf32x8_ps(acc, 1);
		float s = _mm256_hsum_ps(_mm256_add_ps(lo, hi));
		out[r] = bias ? s + bias[r] : s;
	}
}
