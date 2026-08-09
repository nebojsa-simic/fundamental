#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_harness.h"

static int check_int(int condition, TestCount *tc, const char *msg)
{
	if (!condition) {
		printf("\n    FAIL: %s\n", msg);
		tc->failed++;
		return 0;
	}
	tc->passed++;
	return 1;
}

TestCount test_edge_cases(void)
{
	TestCount tc = math_test_count_init();

	printf("\n");

	float nan = _math_test_make_nan();
	float inf = _math_test_make_inf();
	float neg_inf = _math_test_make_neg_inf();
	float neg_zero = _math_test_make_neg_zero();

	/* sqrt edge cases */
	printf("    sqrt: ");
	{
		int start = tc.passed + tc.failed;
		check_int(_math_test_check_same_sign(fun_math_sqrt(0.0f), 0.0f), &tc,
				  "sqrt(0.0) should be +0");
		check_int(_math_test_check_same_sign(fun_math_sqrt(neg_zero), neg_zero),
				  &tc, "sqrt(-0.0) should be -0");
		check_int(_math_test_float_is_inf(fun_math_sqrt(inf)), &tc,
				  "sqrt(+inf) should be +inf");
		check_int(_math_test_float_is_nan(fun_math_sqrt(-1.0f)), &tc,
				  "sqrt(negative) should be NaN");
		check_int(_math_test_float_is_nan(fun_math_sqrt(nan)), &tc,
				  "sqrt(NaN) should be NaN");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* exp edge cases */
	printf("    exp: ");
	{
		int start = tc.passed + tc.failed;
		check_int(fun_math_exp(0.0f) == 1.0f, &tc, "exp(0) should be 1");
		check_int(_math_test_float_is_inf(fun_math_exp(inf)), &tc,
				  "exp(+inf) should be +inf");
		check_int(fun_math_exp(neg_inf) == 0.0f, &tc, "exp(-inf) should be 0");
		check_int(_math_test_float_is_nan(fun_math_exp(nan)), &tc,
				  "exp(NaN) should be NaN");
		check_int(_math_test_float_is_inf(fun_math_exp(100.0f)), &tc,
				  "exp(100) should be +inf (overflow)");
		check_int(fun_math_exp(-100.0f) == 0.0f, &tc,
				  "exp(-100) should be 0 (underflow)");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* log edge cases */
	printf("    log: ");
	{
		int start = tc.passed + tc.failed;
		check_int(fun_math_log(1.0f) == 0.0f, &tc, "log(1) should be 0");
		check_int(_math_test_float_is_inf(fun_math_log(inf)), &tc,
				  "log(+inf) should be +inf");
		check_int(fun_math_log(0.0f) == neg_inf ||
					  _math_test_float_is_inf(fun_math_log(0.0f)) &&
						  fun_math_log(0.0f) < 0.0f,
				  &tc, "log(0.0) should be -inf");
		check_int(fun_math_log(neg_zero) == neg_inf ||
					  _math_test_float_is_inf(fun_math_log(neg_zero)) &&
						  fun_math_log(neg_zero) < 0.0f,
				  &tc, "log(-0.0) should be -inf");
		check_int(_math_test_float_is_nan(fun_math_log(-1.0f)), &tc,
				  "log(negative) should be NaN");
		check_int(_math_test_float_is_nan(fun_math_log(nan)), &tc,
				  "log(NaN) should be NaN");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* sin/cos edge cases */
	printf("    sin/cos: ");
	{
		int start = tc.passed + tc.failed;
		check_int(fun_math_sin(0.0f) == 0.0f, &tc, "sin(0) should be 0");
		check_int(_math_test_float_is_nan(fun_math_sin(nan)), &tc,
				  "sin(NaN) should be NaN");
		check_int(fun_math_cos(0.0f) == 1.0f, &tc, "cos(0) should be 1");
		check_int(_math_test_float_is_nan(fun_math_cos(nan)), &tc,
				  "cos(NaN) should be NaN");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* tanh edge cases */
	printf("    tanh: ");
	{
		int start = tc.passed + tc.failed;
		check_int(fun_math_tanh(0.0f) == 0.0f, &tc, "tanh(0) should be 0");
		check_int(fun_math_tanh(inf) == 1.0f, &tc, "tanh(+inf) should be 1");
		check_int(fun_math_tanh(neg_inf) == -1.0f, &tc,
				  "tanh(-inf) should be -1");
		check_int(_math_test_float_is_nan(fun_math_tanh(nan)), &tc,
				  "tanh(NaN) should be NaN");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* sigmoid edge cases */
	printf("    sigmoid: ");
	{
		int start = tc.passed + tc.failed;
		check_int(fun_math_sigmoid(0.0f) == 0.5f, &tc,
				  "sigmoid(0) should be 0.5");
		float sig_pos = fun_math_sigmoid(20.0f);
		check_int(sig_pos > 0.999f && sig_pos < 1.001f, &tc,
				  "sigmoid(large) should be ~1");
		float sig_neg = fun_math_sigmoid(-20.0f);
		check_int(sig_neg > -0.001f && sig_neg < 0.001f, &tc,
				  "sigmoid(-large) should be ~0");
		check_int(_math_test_float_is_nan(fun_math_sigmoid(nan)), &tc,
				  "sigmoid(NaN) should be NaN");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* zero-length vector calls */
	printf("    zero-length vectors: ");
	{
		int start = tc.passed + tc.failed;
		float buf = 0.0f;
		fun_math_silu_f32(&buf, &buf, 0);
		check_int(1, &tc, "silu_f32(n=0) should return");
		fun_math_rms_norm_f32(&buf, &buf, &buf, 0, 0.0f);
		check_int(1, &tc, "rms_norm_f32(n=0) should return");
		fun_math_swiglu_f32(&buf, &buf, &buf, 0);
		check_int(1, &tc, "swiglu_f32(n=0) should return");
		fun_math_softmax_f32(&buf, 0);
		check_int(1, &tc, "softmax_f32(n=0) should return");
		fun_math_exp_f32(&buf, &buf, 0);
		check_int(1, &tc, "exp_f32(n=0) should return");
		fun_math_log_f32(&buf, &buf, 0);
		check_int(1, &tc, "log_f32(n=0) should return");
		fun_math_sin_f32(&buf, &buf, 0);
		check_int(1, &tc, "sin_f32(n=0) should return");
		fun_math_cos_f32(&buf, &buf, 0);
		check_int(1, &tc, "cos_f32(n=0) should return");
		fun_math_rotary_f32(&buf, &buf, &buf, &buf, 0, 0);
		check_int(1, &tc, "rotary_f32(n_heads=0) should return");
		fun_math_rotary_f32(&buf, &buf, &buf, &buf, 1, 0);
		check_int(1, &tc, "rotary_f32(half=0) should return");
		fun_math_rows_dot_f32(&buf, &buf, &buf, 0, 0, 0, 1.0f);
		check_int(1, &tc, "rows_dot_f32(n_rows=0) should return");
		fun_math_rows_dot_f32(&buf, &buf, &buf, 1, 0, 0, 1.0f);
		check_int(1, &tc, "rows_dot_f32(row_len=0) should return");
		fun_math_weighted_sum_f32(&buf, &buf, &buf, 0, 0, 0);
		check_int(1, &tc, "weighted_sum_f32(n_rows=0) should return");
		fun_math_weighted_sum_f32(&buf, &buf, &buf, 1, 0, 0);
		check_int(1, &tc, "weighted_sum_f32(row_len=0) should return");
		check_int(fun_math_dot_f32(&buf, &buf, 0) == 0.0f, &tc,
				  "dot_f32(n=0) should return 0");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* exp_f32 saturation and special values */
	printf("    exp_f32 special values: ");
	{
		int start = tc.passed + tc.failed;
		float in[8] = { 0.0f, 100.0f, -100.0f, inf, neg_inf, nan, -0.0f, 1.0f };
		float out[8];
		fun_math_exp_f32(in, out, 8);
		check_int(out[0] == 1.0f, &tc, "exp_f32(0) should be 1");
		check_int(_math_test_float_is_inf(out[1]), &tc,
				  "exp_f32(100) should be +inf");
		check_int(out[2] == 0.0f, &tc, "exp_f32(-100) should be 0");
		check_int(_math_test_float_is_inf(out[3]), &tc,
				  "exp_f32(+inf) should be +inf");
		check_int(out[4] == 0.0f, &tc, "exp_f32(-inf) should be 0");
		check_int(_math_test_float_is_nan(out[5]), &tc,
				  "exp_f32(NaN) should be NaN");
		check_int(out[6] == 1.0f, &tc, "exp_f32(-0) should be 1");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* log_f32 special values */
	printf("    log_f32 special values: ");
	{
		int start = tc.passed + tc.failed;
		float in[6] = { 1.0f, 0.0f, neg_zero, -1.0f, inf, nan };
		float out[6];
		fun_math_log_f32(in, out, 6);
		check_int(out[0] == 0.0f, &tc, "log_f32(1) should be 0");
		check_int(_math_test_float_is_inf(out[1]) && out[1] < 0.0f, &tc,
				  "log_f32(0) should be -inf");
		check_int(_math_test_float_is_inf(out[2]) && out[2] < 0.0f, &tc,
				  "log_f32(-0) should be -inf");
		check_int(_math_test_float_is_nan(out[3]), &tc,
				  "log_f32(-1) should be NaN");
		check_int(_math_test_float_is_inf(out[4]) && out[4] > 0.0f, &tc,
				  "log_f32(+inf) should be +inf");
		check_int(_math_test_float_is_nan(out[5]), &tc,
				  "log_f32(NaN) should be NaN");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* sin_f32 / cos_f32 special values */
	printf("    sin/cos_f32 special values: ");
	{
		int start = tc.passed + tc.failed;
		float in[3] = { 0.0f, nan, 0.0f };
		float out[3];
		fun_math_sin_f32(in, out, 2);
		fun_math_cos_f32(in + 2, out + 2, 1);
		check_int(out[0] == 0.0f, &tc, "sin_f32(0) should be 0");
		check_int(_math_test_float_is_nan(out[1]), &tc,
				  "sin_f32(NaN) should be NaN");
		check_int(out[2] == 1.0f, &tc, "cos_f32(0) should be 1");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* matrix_vector_f32 edge cases */
	printf("    matrix_vector_f32 edges: ");
	{
		int start = tc.passed + tc.failed;
		float w[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
		float x[2] = { 1.0f, 1.0f };
		float bias[2] = { 0.5f, -0.5f };
		float out[2];

		fun_math_matrix_vector_f32(w, x, NULL, out, 2, 2);
		check_int(out[0] == 3.0f && out[1] == 7.0f, &tc,
				  "matrix_vector no bias");
		fun_math_matrix_vector_f32(w, x, bias, out, 2, 2);
		check_int(out[0] == 3.5f && out[1] == 6.5f, &tc,
				  "matrix_vector with bias");

		float zero_out[2] = { 99.0f, 99.0f };
		fun_math_matrix_vector_f32(w, x, bias, zero_out, 2, 0);
		check_int(zero_out[0] == 0.5f && zero_out[1] == -0.5f, &tc,
				  "matrix_vector cols=0 yields bias only");
		fun_math_matrix_vector_f32(w, x, NULL, zero_out, 0, 2);
		check_int(1, &tc, "matrix_vector rows=0 should return");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* unaligned buffer handling */
	printf("    unaligned buffers: ");
	{
		int start = tc.passed + tc.failed;
		float aligned[2 + 10];
		float *base = aligned + 1;
		uint32_t rng = _math_test_lcg_seed(55);
		for (int i = 0; i < 10; i++)
			base[i] = _math_test_lcg_float(&rng, -4.0f, 4.0f);

		float *ref = malloc(10 * sizeof(float));
		float *got = malloc(10 * sizeof(float));
		if (!ref || !got) {
			tc.failed++;
		} else {
			fun_math_exp_f32(base, got, 10);
			for (int i = 0; i < 10; i++)
				ref[i] = fun_math_exp(base[i]);
			int ok = 1;
			for (int i = 0; i < 10; i++)
				ok = ok && _math_test_check_float(got[i], ref[i], 1e-4f, 1e-3f);
			check_int(ok, &tc, "exp_f32 on unaligned input matches scalar");

			fun_math_sin_f32(base, got, 10);
			for (int i = 0; i < 10; i++)
				ref[i] = fun_math_sin(base[i]);
			ok = 1;
			for (int i = 0; i < 10; i++)
				ok = ok && _math_test_check_float(got[i], ref[i], 1e-4f, 1e-3f);
			check_int(ok, &tc, "sin_f32 on unaligned input matches scalar");

			float want[10];
			fun_math_sin_f32(base, want, 10);
			fun_math_sin_f32(base + 1, got, 9);
			ok = 1;
			for (int i = 0; i < 9; i++)
				ok = ok &&
					 _math_test_check_float(got[i], want[i + 1], 1e-4f, 1e-3f);
			check_int(ok, &tc, "unaligned input/output match aligned");
		}
		free(ref);
		free(got);
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* rotary_f32 minimal and single-lane cases */
	printf("    rotary_f32 edges: ");
	{
		int start = tc.passed + tc.failed;
		float x[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
		float c[2] = { 1.0f, 0.0f };
		float s[2] = { 0.0f, 1.0f };
		float out[4];

		fun_math_rotary_f32(x, c, s, out, 1, 2);
		check_int(out[0] == 1.0f && out[1] == -4.0f && out[2] == 3.0f &&
					  out[3] == 2.0f,
				  &tc, "rotary half=2 matches rotation formula");

		float one[2] = { 5.0f, 6.0f };
		float c1[1] = { 1.0f };
		float s1[1] = { 0.0f };
		float out1[2];
		fun_math_rotary_f32(one, c1, s1, out1, 1, 1);
		check_int(out1[0] == 5.0f && out1[1] == 6.0f, &tc,
				  "rotary half=1 with cos=1/sin=0 is identity");

		float two_heads[8] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
		float outm[8];
		fun_math_rotary_f32(two_heads, c, s, outm, 2, 2);
		check_int(outm[0] == 1.0f && outm[1] == -4.0f && outm[2] == 3.0f &&
					  outm[3] == 2.0f,
				  &tc, "rotary head 0 does not touch head 1 data");
		check_int(outm[4] == 5.0f && outm[5] == -8.0f && outm[6] == 7.0f &&
					  outm[7] == 6.0f,
				  &tc, "rotary head 1 rotated with shared tables");

		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* rows_dot_f32 minimal and single-row cases */
	printf("    rows_dot_f32 edges: ");
	{
		int start = tc.passed + tc.failed;
		float q[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
		float x[8] = { 3.0f, 7.0f, 7.0f, 7.0f, 2.0f, 5.0f, 8.0f, 1.0f };
		float out[2];

		fun_math_rows_dot_f32(q, x, out, 2, 4, 4, 1.0f);
		check_int(out[0] == 3.0f && out[1] == 2.0f, &tc,
				  "rows_dot row stride 4 picks rows 0 and 4");
		fun_math_rows_dot_f32(q, x, out, 2, 4, 4, 0.0f);
		check_int(out[0] == 0.0f && out[1] == 0.0f, &tc,
				  "rows_dot scale=0 produces zeros");
		fun_math_rows_dot_f32(q, x, out, 1, 4, 4, 2.0f);
		check_int(out[0] == 6.0f, &tc, "rows_dot single row with scale");

		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* weighted_sum_f32 minimal and single-row cases */
	printf("    weighted_sum_f32 edges: ");
	{
		int start = tc.passed + tc.failed;
		float wgt[3] = { 1.0f, 0.0f, 1.0f };
		float x[6] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
		float out[2];
		float col[2] = { 3.5f, -2.0f };

		fun_math_weighted_sum_f32(wgt, x, out, 3, 2, 2);
		check_int(out[0] == 6.0f && out[1] == 8.0f, &tc,
				  "weighted_sum combines rows 0 and 2");
		fun_math_weighted_sum_f32(wgt, col, out, 1, 2, 2);
		check_int(out[0] == col[0] && out[1] == col[1], &tc,
				  "weighted_sum single row copies through");

		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* mxfp4_matvec_f32 edge cases */
	printf("    mxfp4_matvec_f32 edges: ");
	{
		int start = tc.passed + tc.failed;
		const float kv[16] = { 0.0f,  0.5f,	 1.0f,	1.5f,  2.0f,  3.0f,
							   4.0f,  6.0f,	 -0.0f, -0.5f, -1.0f, -1.5f,
							   -2.0f, -3.0f, -4.0f, -6.0f };
		float scale_f = ldexpf(1.0f, 127 - 127); /* scale byte 127 -> 1.0 */
		uint8_t w1[68];
		float x1[64];
		float out[2];

		/* Row 0: scale byte 0 -> whole row decodes to zeros regardless of
		 * nibbles. Row 1: scale byte 127 (factor 1.0); nibble byte j holds
		 * kvalue table[j] on the high nibble, 0 on the low nibble. */
		memset(w1, 0, sizeof(w1));
		for (int j = 0; j < 16; j++)
			w1[35 + j] = (uint8_t)(j << 4); /* low 0, high j */
		w1[34] = 127;
		for (int j = 0; j < 64; j++)
			x1[j] = (float)j;
		float bias1[2] = { 1.0f, 2.0f };

		fun_math_matrix_vector_mxfp4_f32(w1, x1, bias1, out, 2, 64);
		check_int(out[0] == 1.0f, &tc,
				  "mxfp4_matvec zero-scale row yields bias only");
		{
			float expect = 2.0f;
			/* Split order: high nibble j provides element at
			 * position j+16, paired with x1[j+16]. */
			for (int j = 0; j < 16; j++)
				expect += kv[j] * scale_f * x1[j + 16];
			int ok = _math_test_check_float(out[1], expect, 1e-4f, 1e-3f);
			check_int(ok, &tc, "mxfp4_matvec scale 127 kvalue table row");
		}

		/* Scale byte 255 (factor 2^127) with kvalue 0 is exact zero. */
		{
			uint8_t w2[17];
			float x2[32];
			float out2[1];
			memset(w2, 0, sizeof(w2));
			w2[0] = 255;
			for (int j = 0; j < 32; j++)
				x2[j] = 1.0f;
			fun_math_matrix_vector_mxfp4_f32(w2, x2, NULL, out2, 1, 32);
			check_int(out2[0] == 0.0f, &tc,
					  "mxfp4_matvec kvalue 0 with scale 255 is exact zero");
		}

		/* Negative kvalues with a fractional scale: nibble 0xFA decodes to
		 * -1 (even elements) and -6 (odd elements), scale 3 -> 2^-124. */
		{
			uint8_t w3[17];
			float x3[32];
			float out3[1];
			memset(w3, 0, sizeof(w3));
			w3[0] = 3;
			for (int j = 0; j < 16; j++)
				w3[1 + j] = 0xFA;
			for (int j = 0; j < 32; j++)
				x3[j] = 1.0f;
			fun_math_matrix_vector_mxfp4_f32(w3, x3, NULL, out3, 1, 32);
			check_int(out3[0] == -112.0f * ldexpf(1.0f, -124), &tc,
					  "mxfp4_matvec negative kvalues accumulate exactly");
		}

		/* Scale byte 1 decodes to 2^-127 (subnormal when paired with the
		 * x2 lookup); kvalue 0.5 yields exactly 2^-127 per element. */
		{
			uint8_t w4[17];
			float x4[32];
			float out4[1];
			memset(w4, 0, sizeof(w4));
			w4[0] = 1;
			for (int j = 0; j < 16; j++)
				w4[1 + j] = 0x11; /* kvalue 0.5 on both nibbles */
			for (int j = 0; j < 32; j++)
				x4[j] = 1.0f;
			fun_math_matrix_vector_mxfp4_f32(w4, x4, NULL, out4, 1, 32);
			check_int(out4[0] == 32.0f * ldexpf(1.0f, -127), &tc,
					  "mxfp4_matvec scale byte 1 subnormal is exact");
		}

		/* Zero rows must not write out of bounds. */
		{
			float guard[4] = { 9.0f, 9.0f, 9.0f, 9.0f };
			fun_math_matrix_vector_mxfp4_f32(w1, x1, bias1, guard, 0, 64);
			check_int(guard[0] == 9.0f && guard[3] == 9.0f, &tc,
					  "mxfp4_matvec rows=0 leaves output untouched");
		}

		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* fp16_to_f32 edge cases */
	printf("    fp16_to_f32: ");
	{
		int start = tc.passed + tc.failed;
		check_int(fun_math_fp16_to_f32(0x0000) == 0.0f, &tc,
				  "fp16 0x0000 should be 0.0");
		check_int(fun_math_fp16_to_f32(0x8000) == -0.0f &&
					  _math_test_check_same_sign(fun_math_fp16_to_f32(0x8000),
												 _math_test_make_neg_zero()),
				  &tc, "fp16 0x8000 should be -0.0");
		check_int(fun_math_fp16_to_f32(0x3C00) == 1.0f, &tc,
				  "fp16 0x3C00 should be 1.0");
		check_int(fun_math_fp16_to_f32(0xBC00) == -1.0f, &tc,
				  "fp16 0xBC00 should be -1.0");
		check_int(_math_test_float_is_inf(fun_math_fp16_to_f32(0x7C00)), &tc,
				  "fp16 0x7C00 should be +inf");
		check_int(fun_math_fp16_to_f32(0xFC00) == -__builtin_inff(), &tc,
				  "fp16 0xFC00 should be -inf");
		check_int(_math_test_float_is_nan(fun_math_fp16_to_f32(0x7E00)), &tc,
				  "fp16 0x7E00 should be NaN");
		check_int(fun_math_fp16_to_f32(0x0001) == 5.960464477539063e-08f, &tc,
				  "fp16 subnormal 0x0001 exact");
		check_int(fun_math_fp16_to_f32(0x7BFF) == 65504.0f, &tc,
				  "fp16 0x7BFF max value");
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* q8_dequant_f32 edge cases */
	printf("    q8_dequant_f32: ");
	{
		int start = tc.passed + tc.failed;
		{
			uint8_t src[34] = { 0 };
			src[0] = 0x00;
			src[1] = 0x3C;
			for (int j = 0; j < 32; j++)
				src[2 + j] = (uint8_t)(j % 127);
			float out[32];
			fun_math_q8_dequant_row_f32(src, out, 32);
			int ok = 1;
			for (int j = 0; j < 32; j++) {
				if (!_math_test_check_float(out[j], (float)(j % 127) * 1.0f,
											1e-4f, 1e-3f))
					ok = 0;
			}
			check_int(ok, &tc, "q8_dequant scale=1.0 matches int8 values");
		}
		{
			uint8_t src[68] = { 0 };
			src[34] = 0x00;
			src[34 + 1] = 0x40;
			for (int j = 0; j < 32; j++)
				src[34 + 2 + j] = 1;
			float out[64];
			fun_math_q8_dequant_row_f32(src, out, 64);
			check_int(out[0] == 0.0f && out[34] == 2.0f, &tc,
					  "q8_dequant two blocks, second scale=2.0");
		}
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	/* q8_matrix_vector_f32 edge cases */
	printf("    q8_matvec_f32: ");
	{
		int start = tc.passed + tc.failed;
		float guard;
		{
			uint8_t w[34];
			float x[32], out[1], guard;
			memset(w, 0, sizeof(w));
			w[0] = 0x00;
			w[1] = 0x3C;
			for (int j = 0; j < 32; j++) {
				w[2 + j] = (uint8_t)j;
				x[j] = 1.0f;
			}
			fun_math_q8_matrix_vector_f32(w, x, out, 1, 32);
			float expect = 0.0f;
			for (int j = 0; j < 32; j++)
				expect += (float)j;
			check_int(_math_test_check_float(out[0], expect, 1e-4f, 1e-3f), &tc,
					  "q8_matvec single row all-ones x");
		}
		{
			guard = 9.0f;
			fun_math_q8_matrix_vector_f32(NULL, NULL, &guard, 0, 32);
			check_int(guard == 9.0f, &tc,
					  "q8_matvec rows=0 leaves output untouched");
		}
		printf("%d ok\n", (tc.passed + tc.failed) - start);
	}

	return tc;
}
