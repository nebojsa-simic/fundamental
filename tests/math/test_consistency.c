#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "test_harness.h"

static TestCount test_silu_consistency(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(42);
	int n = 1024;
	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *simd = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *scalar = (float *)fun_memory_allocate(n * sizeof(float)).value;

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
	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&simd);
	fun_memory_free((Memory *)&scalar);
	return tc;
}

static TestCount test_rms_consistency(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(99);
	int n = 4096;
	float eps = 1e-5f;
	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *w = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *simd = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *scalar_ref = (float *)fun_memory_allocate(n * sizeof(float)).value;

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
	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&w);
	fun_memory_free((Memory *)&simd);
	fun_memory_free((Memory *)&scalar_ref);
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
	float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *simd = (float *)fun_memory_allocate(n * sizeof(float)).value;
	float *scalar = (float *)fun_memory_allocate(n * sizeof(float)).value;

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
				fun_console_write_line("");
				fun_console_write("      FAIL ");
				fun_console_write(name);
				fun_console_write("[");
				char _buf[32];
				fun_string_from_int(i, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("]: got ");
				char _buf2[64];
				fun_string_from_double((double)simd[i], 6, _buf2,
									   sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)scalar[i], 6, _buf2,
									   sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
				break;
			}
		}
	}

done:
	fun_memory_free((Memory *)&x);
	fun_memory_free((Memory *)&simd);
	fun_memory_free((Memory *)&scalar);
	return tc;
}

TestCount test_consistency(void)
{
	TestCount total = math_test_count_init();

	fun_console_write_line("");
	fun_console_write("    silu_f32 vs scalar: ");
	TestCount tc = test_silu_consistency();
	char _buf[32];
	fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write("/");
	fun_string_from_int(tc.passed + tc.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	if (tc.failed) {
		fun_console_write(" (");
		fun_string_from_int(tc.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" FAILED)");
	}
	fun_console_write_line("");
	math_test_count_merge(&total, tc);

	fun_console_write("    rms_norm_f32 vs scalar: ");
	tc = test_rms_consistency();
	fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write("/");
	fun_string_from_int(tc.passed + tc.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	if (tc.failed) {
		fun_console_write(" (");
		fun_string_from_int(tc.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" FAILED)");
	}
	fun_console_write_line("");
	math_test_count_merge(&total, tc);

	fun_console_write("    exp_f32 vs scalar: ");
	tc = test_unary_consistency("exp_f32", fun_math_exp_f32, fun_math_exp,
								-16.0f, 16.0f, 21);
	fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write("/");
	fun_string_from_int(tc.passed + tc.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	if (tc.failed) {
		fun_console_write(" (");
		fun_string_from_int(tc.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" FAILED)");
	}
	fun_console_write_line("");
	math_test_count_merge(&total, tc);

	fun_console_write("    log_f32 vs scalar: ");
	tc = test_unary_consistency("log_f32", fun_math_log_f32, fun_math_log,
								0.001f, 100.0f, 22);
	fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write("/");
	fun_string_from_int(tc.passed + tc.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	if (tc.failed) {
		fun_console_write(" (");
		fun_string_from_int(tc.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" FAILED)");
	}
	fun_console_write_line("");
	math_test_count_merge(&total, tc);

	fun_console_write("    sin_f32 vs scalar: ");
	tc = test_unary_consistency("sin_f32", fun_math_sin_f32, fun_math_sin,
								-50.0f, 50.0f, 23);
	fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write("/");
	fun_string_from_int(tc.passed + tc.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	if (tc.failed) {
		fun_console_write(" (");
		fun_string_from_int(tc.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" FAILED)");
	}
	fun_console_write_line("");
	math_test_count_merge(&total, tc);

	fun_console_write("    cos_f32 vs scalar: ");
	tc = test_unary_consistency("cos_f32", fun_math_cos_f32, fun_math_cos,
								-50.0f, 50.0f, 24);
	fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write("/");
	fun_string_from_int(tc.passed + tc.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	if (tc.failed) {
		fun_console_write(" (");
		fun_string_from_int(tc.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" FAILED)");
	}
	fun_console_write_line("");
	math_test_count_merge(&total, tc);

	return total;
}
