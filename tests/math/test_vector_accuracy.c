#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_harness.h"

#include "test_data/silu_f32_golden.h"
#include "test_data/rms_norm_f32_golden.h"
#include "test_data/swiglu_f32_golden.h"
#include "test_data/softmax_f32_golden.h"
#include "test_data/exp_f32_golden.h"
#include "test_data/log_f32_golden.h"
#include "test_data/sin_f32_golden.h"
#include "test_data/cos_f32_golden.h"

typedef struct {
	int n;
	const float *x;
	const float *expected;
} BatchUnaryCase;

static TestCount test_batch_unary(const char *name,
								  void (*fn)(const float *, float *, size_t),
								  const BatchUnaryCase *cases)
{
	TestCount tc = math_test_count_init();

	for (int ci = 0; cases[ci].n > 0; ci++) {
		int n = cases[ci].n;
		float *out = malloc((size_t)n * sizeof(float));
		if (!out) {
			tc.failed += n;
			continue;
		}
		fn(cases[ci].x, out, (size_t)n);
		for (int j = 0; j < n; j++) {
			int ok;
			if (_math_test_float_is_nan(out[j]) ||
				_math_test_float_is_nan(cases[ci].expected[j]))
				ok = _math_test_float_is_nan(out[j]) ==
					 _math_test_float_is_nan(cases[ci].expected[j]);
			else
				ok = _math_test_check_float(out[j], cases[ci].expected[j],
											1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				printf("\n      FAIL %s[%d][%d]: got %.6f, expected %.6f\n",
					   name, ci, j, (double)out[j],
					   (double)cases[ci].expected[j]);
			}
		}
		free(out);
	}
	return tc;
}

static TestCount test_mat_vec_accuracy(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(7);

	size_t shapes[][2] = { { 3, 8 }, { 5, 13 },	 { 1, 1 },
						   { 8, 8 }, { 17, 33 }, { 64, 64 } };
	int nshapes = sizeof(shapes) / sizeof(shapes[0]);

	for (int s = 0; s < nshapes; s++) {
		size_t rows = shapes[s][0];
		size_t cols = shapes[s][1];
		float *w = malloc(rows * cols * sizeof(float));
		float *x = malloc(cols * sizeof(float));
		float *bias = malloc(rows * sizeof(float));
		float *got = malloc(rows * sizeof(float));
		float *want = malloc(rows * sizeof(float));
		if (!w || !x || !bias || !got || !want) {
			tc.failed += (int)rows;
			continue;
		}
		for (int trial = 0; trial < 10; trial++) {
			for (size_t i = 0; i < rows * cols; i++)
				w[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
			for (size_t i = 0; i < cols; i++)
				x[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
			for (size_t i = 0; i < rows; i++)
				bias[i] = _math_test_lcg_float(&rng, -0.5f, 0.5f);

			fun_math_matrix_vector_f32(w, x, bias, got, rows, cols);
			fun_math_matrix_vector_f32(w, x, NULL, want, rows, cols);
			for (size_t r = 0; r < rows; r++) {
				float ref = 0.0f;
				for (size_t c = 0; c < cols; c++)
					ref += w[r * cols + c] * x[c];
				int ok =
					_math_test_check_float(got[r], ref + bias[r], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
				ok = _math_test_check_float(want[r], ref, 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
				if (!ok) {
					printf("\n      FAIL mat_vec[%zu/%zu][%zu]: got %.6f, "
						   "expected %.6f\n",
						   rows, cols, r, (double)want[r], (double)ref);
				}
			}
		}
		free(w);
		free(x);
		free(bias);
		free(got);
		free(want);
	}
	return tc;
}

static TestCount test_dot_accuracy(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(13);

	size_t lengths[] = { 1, 7, 8, 9, 64, 1000 };
	int nlen = sizeof(lengths) / sizeof(lengths[0]);

	for (int s = 0; s < nlen; s++) {
		size_t n = lengths[s];
		float *a = malloc(n * sizeof(float));
		float *b = malloc(n * sizeof(float));
		if (!a || !b) {
			tc.failed++;
			continue;
		}
		for (int trial = 0; trial < 10; trial++) {
			for (size_t i = 0; i < n; i++) {
				a[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
				b[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
			}
			float ref = 0.0f;
			for (size_t i = 0; i < n; i++)
				ref += a[i] * b[i];
			float got = fun_math_dot_f32(a, b, n);
			int ok = _math_test_check_float(got, ref, 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				printf("\n      FAIL dot[%zu]: got %.6f, expected %.6f\n", n,
					   (double)got, (double)ref);
			}
		}
		free(a);
		free(b);
	}
	return tc;
}

TestCount test_vector_accuracy(void)
{
	TestCount total = math_test_count_init();

	printf("\n");
	printf("    silu_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; silu_f32_cases[ci].n > 0; ci++) {
			int n = silu_f32_cases[ci].n;
			float *out = malloc(n * sizeof(float));
			if (!out) {
				tc.failed += n;
				continue;
			}
			fun_math_silu_f32(silu_f32_cases[ci].x, out, n);
			for (int j = 0; j < n; j++) {
				int ok = _math_test_check_float(
					out[j], silu_f32_cases[ci].expected[j], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
			}
			free(out);
		}
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    rms_norm_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; rms_norm_f32_cases[ci].n > 0; ci++) {
			int n = rms_norm_f32_cases[ci].n;
			float *out = malloc(n * sizeof(float));
			if (!out) {
				tc.failed += n;
				continue;
			}
			fun_math_rms_norm_f32(rms_norm_f32_cases[ci].x,
								  rms_norm_f32_cases[ci].weight, out, n,
								  rms_norm_f32_cases[ci].eps);
			for (int j = 0; j < n; j++) {
				int ok = _math_test_check_float(
					out[j], rms_norm_f32_cases[ci].expected[j], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
			}
			free(out);
		}
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    swiglu_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; swiglu_f32_cases[ci].n > 0; ci++) {
			int n = swiglu_f32_cases[ci].n;
			float *out = malloc(n * sizeof(float));
			if (!out) {
				tc.failed += n;
				continue;
			}
			fun_math_swiglu_f32(swiglu_f32_cases[ci].gate,
								swiglu_f32_cases[ci].up, out, n);
			for (int j = 0; j < n; j++) {
				int ok = _math_test_check_float(
					out[j], swiglu_f32_cases[ci].expected[j], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
			}
			free(out);
		}
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    softmax_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; softmax_f32_cases[ci].n > 0; ci++) {
			int n = softmax_f32_cases[ci].n;
			float *x = malloc(n * sizeof(float));
			if (!x) {
				tc.failed += n;
				continue;
			}
			memcpy(x, softmax_f32_cases[ci].input, n * sizeof(float));
			fun_math_softmax_f32(x, n);

			float sum = 0.0f;
			for (int j = 0; j < n; j++) {
				int ok = _math_test_check_float(
					x[j], softmax_f32_cases[ci].expected[j], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
				sum += x[j];
			}
			if (!_math_test_check_float(sum, 1.0f, 1e-4f, 1e-4f)) {
				printf("\n      softmax sum=%.9f (not ~1.0)", (double)sum);
				tc.failed++;
			}
			free(x);
		}
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    exp_f32: ");
	{
		TestCount tc = test_batch_unary("exp_f32", fun_math_exp_f32,
										(const BatchUnaryCase *)exp_f32_cases);
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    log_f32: ");
	{
		TestCount tc = test_batch_unary("log_f32", fun_math_log_f32,
										(const BatchUnaryCase *)log_f32_cases);
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    sin_f32: ");
	{
		TestCount tc = test_batch_unary("sin_f32", fun_math_sin_f32,
										(const BatchUnaryCase *)sin_f32_cases);
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    cos_f32: ");
	{
		TestCount tc = test_batch_unary("cos_f32", fun_math_cos_f32,
										(const BatchUnaryCase *)cos_f32_cases);
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    matrix_vector_f32: ");
	{
		TestCount tc = test_mat_vec_accuracy();
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	printf("    dot_f32: ");
	{
		TestCount tc = test_dot_accuracy();
		printf("%d/%d", tc.passed, tc.passed + tc.failed);
		if (tc.failed)
			printf(" (%d FAILED)", tc.failed);
		printf("\n");
		math_test_count_merge(&total, tc);
	}

	return total;
}
