#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "test_harness.h"

#include "test_data/silu_f32_golden.h"
#include "test_data/rms_norm_f32_golden.h"
#include "test_data/swiglu_f32_golden.h"
#include "test_data/softmax_f32_golden.h"
#include "test_data/exp_f32_golden.h"
#include "test_data/log_f32_golden.h"
#include "test_data/sin_f32_golden.h"
#include "test_data/cos_f32_golden.h"
#include "test_data/rotary_f32_golden.h"
#include "test_data/rows_dot_f32_golden.h"
#include "test_data/weighted_sum_f32_golden.h"
#include "test_data/mxfp4_matvec_f32_golden.h"
#include "test_data/fp16_to_f32_golden.h"
#include "test_data/q8_dequant_f32_golden.h"
#include "test_data/q8_matvec_f32_golden.h"

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
		float *out =
			(float *)fun_memory_allocate((size_t)n * sizeof(float)).value;
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
				fun_console_write_line("");
				fun_console_write("      FAIL ");
				fun_console_write(name);
				fun_console_write("[");
				char _buf[32];
				fun_string_from_int(ci, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("][");
				fun_string_from_int(j, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("]: got ");
				char _buf2[64];
				fun_string_from_double((double)out[j], 6, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)cases[ci].expected[j], 6, _buf2,
									   sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
			}
		}
		fun_memory_free((Memory *)&out);
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
		float *w =
			(float *)fun_memory_allocate(rows * cols * sizeof(float)).value;
		float *x = (float *)fun_memory_allocate(cols * sizeof(float)).value;
		float *bias = (float *)fun_memory_allocate(rows * sizeof(float)).value;
		float *got = (float *)fun_memory_allocate(rows * sizeof(float)).value;
		float *want = (float *)fun_memory_allocate(rows * sizeof(float)).value;
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
					fun_console_write_line("");
					fun_console_write("      FAIL mat_vec[");
					char _buf[32];
					fun_string_from_int((int64_t)rows, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("/");
					fun_string_from_int((int64_t)cols, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("][");
					fun_string_from_int((int64_t)r, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("]: got ");
					char _buf2[64];
					fun_string_from_double((double)want[r], 6, _buf2,
										   sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write(", expected ");
					fun_string_from_double((double)ref, 6, _buf2,
										   sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write_line("");
				}
			}
		}
		fun_memory_free((Memory *)&w);
		fun_memory_free((Memory *)&x);
		fun_memory_free((Memory *)&bias);
		fun_memory_free((Memory *)&got);
		fun_memory_free((Memory *)&want);
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
		float *a = (float *)fun_memory_allocate(n * sizeof(float)).value;
		float *b = (float *)fun_memory_allocate(n * sizeof(float)).value;
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
				fun_console_write_line("");
				fun_console_write("      FAIL dot[");
				char _buf[32];
				fun_string_from_int((int64_t)n, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("]: got ");
				char _buf2[64];
				fun_string_from_double((double)got, 6, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)ref, 6, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
			}
		}
		fun_memory_free((Memory *)&a);
		fun_memory_free((Memory *)&b);
	}
	return tc;
}

static void rotary_scalar_ref(const float *x, const float *cosv,
							  const float *sinv, float *out, size_t n_heads,
							  size_t half)
{
	for (size_t h = 0; h < n_heads; h++) {
		const float *xh = x + h * 2 * half;
		float *oh = out + h * 2 * half;
		for (size_t j = 0; j < half; j++) {
			oh[j] = xh[j] * cosv[j] - xh[j + half] * sinv[j];
			oh[j + half] = xh[j] * sinv[j] + xh[j + half] * cosv[j];
		}
	}
}

static TestCount test_rotary_accuracy(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(23);

	size_t halves[] = { 1, 7, 8, 9, 15, 16, 31, 32, 33, 64 };
	int nhalves = sizeof(halves) / sizeof(halves[0]);
	size_t head_counts[] = { 1, 3, 8, 64 };
	int nheads = sizeof(head_counts) / sizeof(head_counts[0]);

	for (int trial = 0; trial < 20; trial++) {
		size_t half = halves[trial % nhalves];
		size_t nh = head_counts[(trial / nhalves) % nheads];
		size_t n = nh * 2 * half;

		float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
		float *c = (float *)fun_memory_allocate(half * sizeof(float)).value;
		float *s = (float *)fun_memory_allocate(half * sizeof(float)).value;
		float *want = (float *)fun_memory_allocate(n * sizeof(float)).value;
		float *got = (float *)fun_memory_allocate(n * sizeof(float)).value;
		if (!x || !c || !s || !want || !got) {
			tc.failed++;
			fun_memory_free((Memory *)&x);
			fun_memory_free((Memory *)&c);
			fun_memory_free((Memory *)&s);
			fun_memory_free((Memory *)&want);
			fun_memory_free((Memory *)&got);
			continue;
		}

		for (size_t i = 0; i < n; i++)
			x[i] = _math_test_lcg_float(&rng, -8.0f, 8.0f);
		for (size_t i = 0; i < half; i++) {
			c[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
			s[i] = _math_test_lcg_float(&rng, -1.0f, 1.0f);
		}

		rotary_scalar_ref(x, c, s, want, nh, half);

		fun_math_rotary_f32(x, c, s, got, nh, half);
		for (size_t i = 0; i < n; i++) {
			int ok = _math_test_check_float(got[i], want[i], 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				fun_console_write_line("");
				fun_console_write("      FAIL rotary out[");
				char _buf[32];
				fun_string_from_int((int64_t)i, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("]: got ");
				char _buf2[64];
				fun_string_from_double((double)got[i], 6, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)want[i], 6, _buf2,
									   sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
			}
		}

		fun_memory_copy(x, got, n * sizeof(float));
		fun_math_rotary_f32(got, c, s, got, nh, half);
		for (size_t i = 0; i < n; i++) {
			int ok = _math_test_check_float(got[i], want[i], 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				fun_console_write_line("");
				fun_console_write("      FAIL rotary in-place[");
				char _buf[32];
				fun_string_from_int((int64_t)i, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("]: got ");
				char _buf2[64];
				fun_string_from_double((double)got[i], 6, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)want[i], 6, _buf2,
									   sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
			}
		}

		fun_memory_free((Memory *)&x);
		fun_memory_free((Memory *)&c);
		fun_memory_free((Memory *)&s);
		fun_memory_free((Memory *)&want);
		fun_memory_free((Memory *)&got);
	}
	return tc;
}

static TestCount test_rows_dot_accuracy(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(29);

	size_t lens[] = { 1, 7, 8, 9, 63, 64 };
	int nlens = sizeof(lens) / sizeof(lens[0]);
	float scales[] = { 1.0f, 0.125f, -2.5f, 0.015625f };
	int nscales = sizeof(scales) / sizeof(scales[0]);

	for (int trial = 0; trial < 24; trial++) {
		size_t rl = lens[trial % nlens];
		size_t nr = (size_t)1 << (2 + (trial / nlens) % 6);
		size_t stride = rl + 3 + (trial % 5);
		float scale = scales[trial % nscales];

		float *q = (float *)fun_memory_allocate(rl * sizeof(float)).value;
		float *x =
			(float *)fun_memory_allocate(stride * nr * sizeof(float)).value;
		float *want = (float *)fun_memory_allocate(nr * sizeof(float)).value;
		float *got = (float *)fun_memory_allocate(nr * sizeof(float)).value;
		if (!q || !x || !want || !got) {
			tc.failed++;
			fun_memory_free((Memory *)&q);
			fun_memory_free((Memory *)&x);
			fun_memory_free((Memory *)&want);
			fun_memory_free((Memory *)&got);
			continue;
		}

		for (size_t i = 0; i < rl; i++)
			q[i] = _math_test_lcg_float(&rng, -8.0f, 8.0f);
		for (size_t i = 0; i < stride * nr; i++)
			x[i] = _math_test_lcg_float(&rng, -8.0f, 8.0f);

		for (size_t t = 0; t < nr; t++) {
			float s = 0.0f;
			for (size_t d = 0; d < rl; d++)
				s += q[d] * x[t * stride + d];
			want[t] = s * scale;
		}

		fun_math_rows_dot_f32(q, x, got, nr, rl, stride, scale);
		for (size_t t = 0; t < nr; t++) {
			int ok = _math_test_check_float(got[t], want[t], 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				fun_console_write_line("");
				fun_console_write("      FAIL rows_dot[");
				char _buf[32];
				fun_string_from_int((int64_t)nr, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("][");
				fun_string_from_int((int64_t)t, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("]: got ");
				char _buf2[64];
				fun_string_from_double((double)got[t], 6, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)want[t], 6, _buf2,
									   sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
			}
		}

		fun_memory_free((Memory *)&q);
		fun_memory_free((Memory *)&x);
		fun_memory_free((Memory *)&want);
		fun_memory_free((Memory *)&got);
	}
	return tc;
}

static TestCount test_weighted_sum_accuracy(void)
{
	TestCount tc = math_test_count_init();
	uint32_t rng = _math_test_lcg_seed(31);

	size_t lens[] = { 1, 7, 8, 9, 63, 64 };
	int nlens = sizeof(lens) / sizeof(lens[0]);

	for (int trial = 0; trial < 24; trial++) {
		size_t rl = lens[trial % nlens];
		size_t nr = (size_t)1 << (2 + (trial / nlens) % 6);
		size_t stride = rl + 2 + (trial % 7);

		float *wgt = (float *)fun_memory_allocate(nr * sizeof(float)).value;
		float *x =
			(float *)fun_memory_allocate(stride * nr * sizeof(float)).value;
		float *want = (float *)fun_memory_allocate(rl * sizeof(float)).value;
		float *got = (float *)fun_memory_allocate(rl * sizeof(float)).value;
		if (!wgt || !x || !want || !got) {
			tc.failed++;
			fun_memory_free((Memory *)&wgt);
			fun_memory_free((Memory *)&x);
			fun_memory_free((Memory *)&want);
			fun_memory_free((Memory *)&got);
			continue;
		}

		for (size_t i = 0; i < nr; i++)
			wgt[i] = _math_test_lcg_float(&rng, -2.0f, 2.0f);
		for (size_t i = 0; i < stride * nr; i++)
			x[i] = _math_test_lcg_float(&rng, -8.0f, 8.0f);

		for (size_t d = 0; d < rl; d++) {
			float s = 0.0f;
			for (size_t t = 0; t < nr; t++)
				s += wgt[t] * x[t * stride + d];
			want[d] = s;
		}

		for (size_t d = 0; d < rl; d++)
			got[d] = 1234.0f;
		fun_math_weighted_sum_f32(wgt, x, got, nr, rl, stride);
		for (size_t d = 0; d < rl; d++) {
			int ok = _math_test_check_float(got[d], want[d], 1e-4f, 1e-3f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				fun_console_write_line("");
				fun_console_write("      FAIL weighted_sum[");
				char _buf[32];
				fun_string_from_int((int64_t)nr, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("][");
				fun_string_from_int((int64_t)d, 10, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write("]: got ");
				char _buf2[64];
				fun_string_from_double((double)got[d], 6, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)want[d], 6, _buf2,
									   sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
			}
		}

		fun_memory_free((Memory *)&wgt);
		fun_memory_free((Memory *)&x);
		fun_memory_free((Memory *)&want);
		fun_memory_free((Memory *)&got);
	}
	return tc;
}

TestCount test_vector_accuracy(void)
{
	TestCount total = math_test_count_init();

	fun_console_write_line("");
	fun_console_write("    silu_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; silu_f32_cases[ci].n > 0; ci++) {
			int n = silu_f32_cases[ci].n;
			float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
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
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    rms_norm_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; rms_norm_f32_cases[ci].n > 0; ci++) {
			int n = rms_norm_f32_cases[ci].n;
			float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
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
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    swiglu_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; swiglu_f32_cases[ci].n > 0; ci++) {
			int n = swiglu_f32_cases[ci].n;
			float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
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
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    softmax_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; softmax_f32_cases[ci].n > 0; ci++) {
			int n = softmax_f32_cases[ci].n;
			float *x = (float *)fun_memory_allocate(n * sizeof(float)).value;
			if (!x) {
				tc.failed += n;
				continue;
			}
			fun_memory_copy((Memory)softmax_f32_cases[ci].input, (Memory)x,
							n * sizeof(float));
			fun_math_softmax_f32(x, n);

			float sum = 0.0f;
			for (int j = 0; j < n; j++) {
				int ok = _math_test_check_float(
					x[j], softmax_f32_cases[ci].expected[j], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
				sum += x[j];
			}
			if (!_math_test_check_float(sum, 1.0f, 1e-4f, 1e-4f)) {
				fun_console_write_line("");
				fun_console_write("      softmax sum=");
				char _buf2[64];
				fun_string_from_double((double)sum, 9, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(" (not ~1.0)");
				tc.failed++;
			}
			fun_memory_free((Memory *)&x);
		}
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
	}

	fun_console_write("    exp_f32: ");
	{
		TestCount tc = test_batch_unary("exp_f32", fun_math_exp_f32,
										(const BatchUnaryCase *)exp_f32_cases);
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
	}

	fun_console_write("    log_f32: ");
	{
		TestCount tc = test_batch_unary("log_f32", fun_math_log_f32,
										(const BatchUnaryCase *)log_f32_cases);
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
	}

	fun_console_write("    sin_f32: ");
	{
		TestCount tc = test_batch_unary("sin_f32", fun_math_sin_f32,
										(const BatchUnaryCase *)sin_f32_cases);
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
	}

	fun_console_write("    cos_f32: ");
	{
		TestCount tc = test_batch_unary("cos_f32", fun_math_cos_f32,
										(const BatchUnaryCase *)cos_f32_cases);
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
	}

	fun_console_write("    rotary_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; rotary_f32_cases[ci].n_heads > 0; ci++) {
			size_t nh = (size_t)rotary_f32_cases[ci].n_heads;
			size_t half = (size_t)rotary_f32_cases[ci].half;
			size_t n = nh * 2 * half;
			float *out = (float *)fun_memory_allocate(n * sizeof(float)).value;
			if (!out) {
				tc.failed += (int)n;
				continue;
			}
			fun_math_rotary_f32(rotary_f32_cases[ci].x,
								rotary_f32_cases[ci].cosv,
								rotary_f32_cases[ci].sinv, out, nh, half);
			for (size_t j = 0; j < n; j++) {
				int ok = _math_test_check_float(
					out[j], rotary_f32_cases[ci].expected[j], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
				if (!ok) {
					fun_console_write_line("");
					fun_console_write("      FAIL rotary golden[");
					char _buf[32];
					fun_string_from_int(ci, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("][");
					fun_string_from_int((int64_t)j, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("]: got ");
					char _buf2[64];
					fun_string_from_double((double)out[j], 6, _buf2,
										   sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write(", expected ");
					fun_string_from_double(
						(double)rotary_f32_cases[ci].expected[j], 6, _buf2,
						sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write_line("");
				}
			}
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    rotary_f32 sweep: ");
	{
		TestCount tc = test_rotary_accuracy();
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
	}

	fun_console_write("    rows_dot_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; rows_dot_f32_cases[ci].row_stride != 0; ci++) {
			size_t nr = (size_t)rows_dot_f32_cases[ci].n_rows;
			size_t rl = (size_t)rows_dot_f32_cases[ci].row_len;
			float *out = (float *)fun_memory_allocate(nr * sizeof(float)).value;
			if (!out) {
				tc.failed += (int)nr;
				continue;
			}
			fun_math_rows_dot_f32(rows_dot_f32_cases[ci].q,
								  rows_dot_f32_cases[ci].x, out, nr, rl,
								  (size_t)rows_dot_f32_cases[ci].row_stride,
								  rows_dot_f32_cases[ci].scale);
			for (size_t t = 0; t < nr; t++) {
				int ok = _math_test_check_float(
					out[t], rows_dot_f32_cases[ci].expected[t], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
				if (!ok) {
					fun_console_write_line("");
					fun_console_write("      FAIL rows_dot golden[");
					char _buf[32];
					fun_string_from_int(ci, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("][");
					fun_string_from_int((int64_t)t, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("]: got ");
					char _buf2[64];
					fun_string_from_double((double)out[t], 6, _buf2,
										   sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write(", expected ");
					fun_string_from_double(
						(double)rows_dot_f32_cases[ci].expected[t], 6, _buf2,
						sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write_line("");
				}
			}
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    rows_dot_f32 sweep: ");
	{
		TestCount tc = test_rows_dot_accuracy();
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
	}

	fun_console_write("    weighted_sum_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; weighted_sum_f32_cases[ci].row_stride != 0; ci++) {
			size_t nr = (size_t)weighted_sum_f32_cases[ci].n_rows;
			size_t rl = (size_t)weighted_sum_f32_cases[ci].row_len;
			float *out = (float *)fun_memory_allocate(rl * sizeof(float)).value;
			if (!out) {
				tc.failed += (int)rl;
				continue;
			}
			fun_math_weighted_sum_f32(
				weighted_sum_f32_cases[ci].wgt, weighted_sum_f32_cases[ci].x,
				out, nr, rl, (size_t)weighted_sum_f32_cases[ci].row_stride);
			for (size_t d = 0; d < rl; d++) {
				int ok = _math_test_check_float(
					out[d], weighted_sum_f32_cases[ci].expected[d], 1e-4f,
					1e-3f);
				math_test_count_add(&tc, ok);
				if (!ok) {
					fun_console_write_line("");
					fun_console_write("      FAIL weighted_sum golden[");
					char _buf[32];
					fun_string_from_int(ci, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("][");
					fun_string_from_int((int64_t)d, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("]: got ");
					char _buf2[64];
					fun_string_from_double((double)out[d], 6, _buf2,
										   sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write(", expected ");
					fun_string_from_double(
						(double)weighted_sum_f32_cases[ci].expected[d], 6,
						_buf2, sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write_line("");
				}
			}
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    weighted_sum_f32 sweep: ");
	{
		TestCount tc = test_weighted_sum_accuracy();
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
	}

	fun_console_write("    matrix_vector_f32: ");
	{
		TestCount tc = test_mat_vec_accuracy();
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
	}

	fun_console_write("    mxfp4_matvec_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; mxfp4_matvec_f32_cases[ci].cols != 0; ci++) {
			size_t nr = (size_t)mxfp4_matvec_f32_cases[ci].n_rows;
			size_t cols = (size_t)mxfp4_matvec_f32_cases[ci].cols;
			float *out =
				(float *)fun_memory_allocate((nr > 0 ? nr : 1) * sizeof(float))
					.value;
			if (!out) {
				tc.failed += (int)nr;
				continue;
			}
			fun_math_matrix_vector_mxfp4_f32(mxfp4_matvec_f32_cases[ci].w,
											 mxfp4_matvec_f32_cases[ci].x,
											 mxfp4_matvec_f32_cases[ci].bias,
											 out, nr, cols);
			for (size_t t = 0; t < nr; t++) {
				int ok = _math_test_check_float(
					out[t], mxfp4_matvec_f32_cases[ci].expected[t],
					mxfp4_matvec_f32_cases[ci].abs_tol, 1e-3f);
				math_test_count_add(&tc, ok);
				if (!ok) {
					fun_console_write_line("");
					fun_console_write("      FAIL mxfp4_matvec golden[");
					char _buf[32];
					fun_string_from_int(ci, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("][");
					fun_string_from_int((int64_t)t, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("]: got ");
					char _buf2[64];
					fun_string_from_double((double)out[t], 6, _buf2,
										   sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write(", expected ");
					fun_string_from_double(
						(double)mxfp4_matvec_f32_cases[ci].expected[t], 6,
						_buf2, sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write_line("");
				}
			}
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    dot_f32: ");
	{
		TestCount tc = test_dot_accuracy();
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
	}

	fun_console_write("    fp16_to_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; fp16_to_f32_cases[ci].abs_tol >= 0.0f; ci++) {
			uint16_t h = (uint16_t)(int)fp16_to_f32_cases[ci].input;
			float got = fun_math_fp16_to_f32(h);
			float want = fp16_to_f32_cases[ci].expected;
			int ok;
			if (_math_test_float_is_nan(got) || _math_test_float_is_nan(want))
				ok = _math_test_float_is_nan(got) ==
					 _math_test_float_is_nan(want);
			else
				ok = (got == want) ||
					 _math_test_check_float(got, want, 1e-7f, 1e-7f);
			math_test_count_add(&tc, ok);
			if (!ok) {
				fun_console_write_line("");
				fun_console_write("      FAIL fp16_to_f32 0x");
				char _buf[32];
				fun_string_from_int((unsigned)h, 16, _buf, sizeof(_buf));
				fun_console_write(_buf);
				fun_console_write(": got ");
				char _buf2[64];
				fun_string_from_double((double)got, 9, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write(", expected ");
				fun_string_from_double((double)want, 9, _buf2, sizeof(_buf2));
				fun_console_write(_buf2);
				fun_console_write_line("");
			}
		}
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
	}

	fun_console_write("    q8_dequant_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; q8_dequant_f32_cases[ci].n > 0; ci++) {
			int n = q8_dequant_f32_cases[ci].n;
			float *out =
				(float *)fun_memory_allocate((size_t)n * sizeof(float)).value;
			if (!out) {
				tc.failed += n;
				continue;
			}
			fun_math_q8_dequant_row_f32(q8_dequant_f32_cases[ci].src, out,
										(size_t)n);
			for (int j = 0; j < n; j++) {
				int ok = _math_test_check_float(
					out[j], q8_dequant_f32_cases[ci].expected[j], 1e-4f, 1e-3f);
				math_test_count_add(&tc, ok);
			}
			fun_memory_free((Memory *)&out);
		}
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
	}

	fun_console_write("    q8_matvec_f32: ");
	{
		TestCount tc = math_test_count_init();
		for (int ci = 0; q8_matvec_f32_cases[ci].cols != 0; ci++) {
			size_t nr = (size_t)q8_matvec_f32_cases[ci].n_rows;
			size_t cols = (size_t)q8_matvec_f32_cases[ci].cols;
			float *out =
				(float *)fun_memory_allocate((nr > 0 ? nr : 1) * sizeof(float))
					.value;
			if (!out) {
				tc.failed += (int)nr;
				continue;
			}
			fun_math_q8_matrix_vector_f32(q8_matvec_f32_cases[ci].w,
										  q8_matvec_f32_cases[ci].x, out, nr,
										  cols);
			for (size_t t = 0; t < nr; t++) {
				int ok = _math_test_check_float(
					out[t], q8_matvec_f32_cases[ci].expected[t],
					q8_matvec_f32_cases[ci].abs_tol, 1e-3f);
				math_test_count_add(&tc, ok);
				if (!ok) {
					fun_console_write_line("");
					fun_console_write("      FAIL q8_matvec golden[");
					char _buf[32];
					fun_string_from_int(ci, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("][");
					fun_string_from_int((int64_t)t, 10, _buf, sizeof(_buf));
					fun_console_write(_buf);
					fun_console_write("]: got ");
					char _buf2[64];
					fun_string_from_double((double)out[t], 6, _buf2,
										   sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write(", expected ");
					fun_string_from_double(
						(double)q8_matvec_f32_cases[ci].expected[t], 6, _buf2,
						sizeof(_buf2));
					fun_console_write(_buf2);
					fun_console_write_line("");
				}
			}
			fun_memory_free((Memory *)&out);
		}
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
	}

	return total;
}
