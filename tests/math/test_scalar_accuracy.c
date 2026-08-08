#include <math.h>
#include <stdio.h>
#include "test_harness.h"

#include "test_data/sqrt_golden.h"
#include "test_data/exp_golden.h"
#include "test_data/log_golden.h"
#include "test_data/sin_golden.h"
#include "test_data/cos_golden.h"
#include "test_data/tanh_golden.h"
#include "test_data/sigmoid_golden.h"
#include "test_data/silu_golden.h"

typedef float (*ScalarFn)(float);
typedef struct {
	const char *name;
	ScalarFn fn;
	float rel_tol;
} ScalarTest;

static TestCount run_1d_test(const char *name, ScalarFn fn, const void *cases,
							 size_t stride, float rel_tol)
{
	TestCount tc = math_test_count_init();
	const char *ptr = (const char *)cases;

	for (int i = 0;; i++) {
		float input = *(const float *)(ptr + i * stride);
		float expected = *(const float *)(ptr + i * stride + 4);
		float abs_tol = *(const float *)(ptr + i * stride + 8);

		if (input == 0.0f && expected == 0.0f && abs_tol == 0.0f && i > 0)
			break;

		float got = fn(input);
		int ok;
		if (_math_test_float_is_nan(got) || _math_test_float_is_nan(expected))
			ok = _math_test_float_is_nan(got) ==
				 _math_test_float_is_nan(expected);
		else
			ok = _math_test_check_float(got, expected, abs_tol, rel_tol);
		math_test_count_add(&tc, ok);

		if (!ok) {
			printf("\n    FAIL %s(%.4f): got %.6f, expected %.6f\n", name,
				   (double)input, (double)got, (double)expected);
		}
	}
	return tc;
}

TestCount test_scalar_accuracy(void)
{
	TestCount total = math_test_count_init();

	printf("\n");
	TestCount tc = run_1d_test("sqrt", fun_math_sqrt, sqrt_cases,
							   sizeof(sqrt_case), 1e-4f);
	printf("    sqrt: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	tc = run_1d_test("exp", fun_math_exp, exp_cases, sizeof(exp_case), 1e-3f);
	printf("    exp: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	tc = run_1d_test("log", fun_math_log, log_cases, sizeof(log_case), 1e-3f);
	printf("    log: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	tc = run_1d_test("sin", fun_math_sin, sin_cases, sizeof(sin_case), 1e-3f);
	printf("    sin: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	tc = run_1d_test("cos", fun_math_cos, cos_cases, sizeof(cos_case), 1e-3f);
	printf("    cos: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	tc = run_1d_test("tanh", fun_math_tanh, tanh_cases, sizeof(tanh_case),
					 1e-3f);
	printf("    tanh: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	tc = run_1d_test("sigmoid", fun_math_sigmoid, sigmoid_cases,
					 sizeof(sigmoid_case), 1e-3f);
	printf("    sigmoid: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	tc = run_1d_test("silu", fun_math_silu, silu_cases, sizeof(silu_case),
					 1e-3f);
	printf("    silu: %d/%d", tc.passed, tc.passed + tc.failed);
	if (tc.failed)
		printf(" (%d FAILED)", tc.failed);
	printf("\n");
	math_test_count_merge(&total, tc);

	return total;
}
