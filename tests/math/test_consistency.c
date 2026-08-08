#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_harness.h"

static TestCount test_silu_consistency(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(42);
	int n = 1024;
	float *x = malloc(n * sizeof(float));
	float *simd = malloc(n * sizeof(float));
	float *scalar = malloc(n * sizeof(float));

	if (!x || !simd || !scalar) {
		tc.failed = 1;
		goto done;
	}

	for (int trial = 0; trial < 100; trial++) {
		for (int i = 0; i < n; i++)
			x[i] = _math_test_lcg_float(&rng, -8.0f, 8.0f);

		fun_math_silu_f32(x, simd, n);
		for (int i = 0; i < n; i++)
			scalar[i] = fun_math_silu(x[i]);

		for (int i = 0; i < n; i++) {
			int ok = _math_test_check_float(simd[i], scalar[i], 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
		}
	}

done:
	free(x);
	free(simd);
	free(scalar);
	return tc;
}

static TestCount test_rms_consistency(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(99);
	int n = 4096;
	float eps = 1e-5f;
	float *x = malloc(n * sizeof(float));
	float *w = malloc(n * sizeof(float));
	float *simd = malloc(n * sizeof(float));
	float *scalar_ref = malloc(n * sizeof(float));

	if (!x || !w || !simd || !scalar_ref) {
		tc.failed = 1;
		goto done;
	}

	for (int trial = 0; trial < 10; trial++) {
		float ss = 0.0f;
		for (int i = 0; i < n; i++) {
			x[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
			w[i] = _math_test_lcg_float(&rng, 0.5f, 1.5f);
			ss += x[i] * x[i];
		}

		fun_math_rms_norm_f32(x, w, simd, n, eps);

		float inv_rms = 1.0f / fun_math_sqrt(ss / (float)n + eps);
		for (int i = 0; i < n; i++)
			scalar_ref[i] = x[i] * inv_rms * w[i];

		for (int i = 0; i < n; i++) {
			int ok =
				_math_test_check_float(simd[i], scalar_ref[i], 5e-4f, 5e-3f);
			math_test_count_add(&tc, ok);
		}
	}

done:
	free(x);
	free(w);
	free(simd);
	free(scalar_ref);
	return tc;
}

static TestCount
test_unary_consistency(const char *name,
					   void (*fn)(const float *, float *, size_t),
					   float (*sfn)(float), float lo, float hi, uint32_t seed)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(seed);
	int n = 1024;
	float *x = malloc(n * sizeof(float));
	float *simd = malloc(n * sizeof(float));
	float *scalar = malloc(n * sizeof(float));

	if (!x || !simd || !scalar) {
		tc.failed = 1;
		goto done;
	}

	for (int trial = 0; trial < 100; trial++) {
		for (int i = 0; i < n; i++)
			x[i] = _math_test_lcg_float(&rng, lo, hi);

		fn(x, simd, n);
		for (int i = 0; i < n; i++)
			scalar[i] = sfn(x[i]);

		for (int i = 0; i < n; i++) {
			int ok;
			if (_math_test_float_is_nan(simd[i]) ||
				_math_test_float_is_nan(scalar[i]))
				ok = _math_test_float_is_nan(simd[i]) ==
					 _math_test_float_is_nan(scalar[i]);
			else
				ok = _math_test_check_float(simd[i], scalar[i], 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				printf("\n      FAIL %s[%d]: got %.6f, expected %.6f\n", name,
					   i, (double)simd[i], (double)scalar[i]);
				break;
			}
		}
	}

done:
	free(x);
	free(simd);
	free(scalar);
	return tc;
}

TestCount test_consistency(void)
{
	TestCount total = math_test_count_init();

	printf("\n");
	printf("    silu_f32 vs scalar: ");
	TestCount tc = test_silu_consistency();
	printf("%d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	printf("    rms_norm_f32 vs scalar: ");
	tc = test_rms_consistency();
	printf("%d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	printf("    exp_f32 vs scalar: ");
	tc = test_unary_consistency("exp_f32", fun_math_exp_f32, fun_math_exp,
								-16.0f, 16.0f, 21);
	printf("%d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	printf("    log_f32 vs scalar: ");
	tc = test_unary_consistency("log_f32", fun_math_log_f32, fun_math_log,
								0.001f, 100.0f, 22);
	printf("%d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	printf("    sin_f32 vs scalar: ");
	tc = test_unary_consistency("sin_f32", fun_math_sin_f32, fun_math_sin,
								-50.0f, 50.0f, 23);
	printf("%d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	printf("    cos_f32 vs scalar: ");
	tc = test_unary_consistency("cos_f32", fun_math_cos_f32, fun_math_cos,
								-50.0f, 50.0f, 24);
	printf("%d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	return total;
}
