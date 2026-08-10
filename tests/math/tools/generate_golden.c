#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_DIR "../test_data"

static void f32_str(float x, char *buf, size_t sz)
{
	uint32_t u = *(uint32_t *)&x;
	if ((u & 0x7F800000u) == 0x7F800000u && (u & 0x007FFFFFu) != 0) {
		snprintf(buf, sz, "__builtin_nanf(\"\")");
		return;
	}
	if (u == 0x7F800000u) {
		snprintf(buf, sz, "__builtin_inff()");
		return;
	}
	if (u == 0xFF800000u) {
		snprintf(buf, sz, "-__builtin_inff()");
		return;
	}
	if (u == 0x80000000u) {
		snprintf(buf, sz, "-0.0f");
		return;
	}
	if (x == (float)(int)x && u != 0x80000000u) {
		snprintf(buf, sz, "%.1ff", x);
		return;
	}
	snprintf(buf, sz, "%.9gf", x);
}

static float frand(void)
{
	return (float)rand() / (float)RAND_MAX;
}

static float sigmoidf(float x)
{
	return 1.0f / (1.0f + expf(-x));
}

static float siluf(float x)
{
	return x * sigmoidf(x);
}

static void write_1d_scalar(const char *name, int n, const float *inputs,
							const float *expected, const float *abs_tols)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/%s_golden.h", OUTPUT_DIR, name);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f,
			"typedef struct { float input; float expected; float abs_tol; "
			"} %s_case;\n",
			name);
	fprintf(f, "static const %s_case %s_cases[] = {\n", name, name);

	char ia[32], ea[32];
	for (int i = 0; i < n; i++) {
		f32_str(inputs[i], ia, sizeof(ia));
		f32_str(expected[i], ea, sizeof(ea));
		fprintf(f, "    { %s, %s, %.9g },\n", ia, ea, abs_tols[i]);
	}
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");

	fclose(f);
	printf("  %s_golden.h (%d cases)\n", name, n);
}

static void gen_linspace(float lo, float hi, int n, float *out)
{
	for (int i = 0; i < n; i++)
		out[i] = lo + (hi - lo) * (float)i / (float)(n - 1);
}

static void gen_sqrt(void)
{
	int n = 0;
	float in[2500], ex[2500], at[2500];

	float base[] = {
		0.0f,	   1.0f,	 2.0f,		3.0f,		 4.0f,	5.0f,
		8.0f,	   9.0f,	 10.0f,		16.0f,		 25.0f, 36.0f,
		49.0f,	   64.0f,	 81.0f,		100.0f,		 0.25f, 0.01f,
		0.000001f, 10000.0f, 1000000.f, 3.14159265f, 0.5f,	7.0f,
		0.0625f,   121.0f,	 144.0f,	1.5f,		 6.25f,
	};
	for (size_t i = 0; i < sizeof(base) / sizeof(base[0]); i++) {
		in[n] = base[i];
		ex[n] = sqrtf(in[n]);
		at[n] = 1e-5f;
		n++;
	}

	float sweep[2000];
	gen_linspace(0.001f, 1e6f, 2000, sweep);
	for (int i = 0; i < 2000; i++) {
		in[n] = sweep[i];
		ex[n] = sqrtf(in[n]);
		at[n] = 1e-4f;
		n++;
	}

	write_1d_scalar("sqrt", n, in, ex, at);
}

static void gen_exp(void)
{
	int n = 0;
	float in[2500], ex[2500], at[2500];
	float b[] = { 0.0f,	 1.0f,	 -1.0f, 2.0f,	-2.0f, 5.0f, -5.0f,
				  10.0f, -10.0f, 20.0f, -20.0f, 0.5f,  -0.5f };
	for (size_t i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		in[n] = b[i];
		ex[n] = expf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	float sweep[2000];
	gen_linspace(-88.0f, 88.0f, 2000, sweep);
	for (int i = 0; i < 2000; i++) {
		in[n] = sweep[i];
		ex[n] = expf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	write_1d_scalar("exp", n, in, ex, at);
}

static void gen_log(void)
{
	int n = 0;
	float in[2500], ex[2500], at[2500];
	float b[] = { 0.001f, 0.01f,	   0.1f,  0.5f,	  1.0f,
				  2.0f,	  2.71828183f, 10.0f, 100.0f, 1e6f };
	for (size_t i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		in[n] = b[i];
		ex[n] = logf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	float sweep[2000];
	gen_linspace(0.001f, 1e6f, 2000, sweep);
	for (int i = 0; i < 2000; i++) {
		in[n] = sweep[i];
		ex[n] = logf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	write_1d_scalar("log", n, in, ex, at);
}

static void gen_sin(void)
{
	int n = 0;
	float in[2500], ex[2500], at[2500];
	float b[] = { 0.0f,
				  (float)(M_PI / 6),
				  (float)(M_PI / 4),
				  (float)(M_PI / 3),
				  (float)(M_PI / 2),
				  (float)M_PI,
				  (float)(3 * M_PI / 2),
				  (float)(2 * M_PI),
				  (float)(-M_PI / 2),
				  (float)(-M_PI),
				  1.0f,
				  -1.0f };
	for (size_t i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		in[n] = b[i];
		ex[n] = sinf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	float sweep[2000];
	gen_linspace(-50.0f, 50.0f, 2000, sweep);
	for (int i = 0; i < 2000; i++) {
		in[n] = sweep[i];
		ex[n] = sinf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	write_1d_scalar("sin", n, in, ex, at);
}

static void gen_cos(void)
{
	int n = 0;
	float in[2500], ex[2500], at[2500];
	float b[] = { 0.0f,
				  (float)(M_PI / 6),
				  (float)(M_PI / 4),
				  (float)(M_PI / 3),
				  (float)(M_PI / 2),
				  (float)M_PI,
				  (float)(3 * M_PI / 2),
				  (float)(2 * M_PI),
				  (float)(-M_PI / 2),
				  (float)(-M_PI) };
	for (size_t i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		in[n] = b[i];
		ex[n] = cosf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	float sweep[2000];
	gen_linspace(-50.0f, 50.0f, 2000, sweep);
	for (int i = 0; i < 2000; i++) {
		in[n] = sweep[i];
		ex[n] = cosf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	write_1d_scalar("cos", n, in, ex, at);
}

static void gen_tanh(void)
{
	int n = 0;
	float in[1500], ex[1500], at[1500];
	float b[] = { -4.0f, -3.0f, -2.0f, -1.0f, -0.5f, 0.0f,
				  0.5f,	 1.0f,	2.0f,  3.0f,  4.0f };
	for (size_t i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		in[n] = b[i];
		ex[n] = tanhf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	float sweep[1000];
	gen_linspace(-5.0f, 5.0f, 1000, sweep);
	for (int i = 0; i < 1000; i++) {
		in[n] = sweep[i];
		ex[n] = tanhf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	write_1d_scalar("tanh", n, in, ex, at);
}

static void gen_sigmoid(void)
{
	int n = 0;
	float in[1500], ex[1500], at[1500];
	float b[] = { -8.0f, -6.0f, -4.0f, -3.0f, -2.0f, -1.0f, -0.5f, 0.0f,
				  0.5f,	 1.0f,	2.0f,  3.0f,  4.0f,	 6.0f,	8.0f };
	for (size_t i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		in[n] = b[i];
		ex[n] = sigmoidf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	float sweep[1000];
	gen_linspace(-10.0f, 10.0f, 1000, sweep);
	for (int i = 0; i < 1000; i++) {
		in[n] = sweep[i];
		ex[n] = sigmoidf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	write_1d_scalar("sigmoid", n, in, ex, at);
}

static void gen_silu(void)
{
	int n = 0;
	float in[2500], ex[2500], at[2500];
	float b[] = { -8.0f, -6.0f, -4.0f, -3.0f, -2.0f, -1.0f, -0.5f, 0.0f,
				  0.5f,	 1.0f,	2.0f,  3.0f,  4.0f,	 6.0f,	8.0f };
	for (size_t i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		in[n] = b[i];
		ex[n] = siluf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	float sweep[2000];
	gen_linspace(-10.0f, 10.0f, 2000, sweep);
	for (int i = 0; i < 2000; i++) {
		in[n] = sweep[i];
		ex[n] = siluf(in[n]);
		at[n] = 1e-4f;
		n++;
	}
	write_1d_scalar("silu", n, in, ex, at);
}

static void gen_vector_silu_f32(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/silu_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n; const float *x; const float *expected; "
			   "} silu_f32_case;\n\n");

	int sizes[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	int nsizes = sizeof(sizes) / sizeof(sizes[0]);
	int saved_n[50];

	for (int ci = 0; ci < 50; ci++) {
		int nz = sizes[rand() % nsizes];
		saved_n[ci] = nz;
		float *x = malloc(nz * sizeof(float));
		float *e = malloc(nz * sizeof(float));
		for (int j = 0; j < nz; j++) {
			x[j] = (frand() - 0.5f) * 16.0f;
			e[j] = siluf(x[j]);
		}

		fprintf(f, "static float _silu_x_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(x[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _silu_e_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(x);
		free(e);
	}

	fprintf(f, "\nstatic const silu_f32_case silu_f32_cases[] = {\n");
	for (int ci = 0; ci < 50; ci++)
		fprintf(f, "    { %d, _silu_x_%d, _silu_e_%d },\n", saved_n[ci], ci,
				ci);
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  silu_f32_golden.h (50 cases)\n");
}

static void gen_vector_rms_norm(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/rms_norm_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n; float eps; const float *x; const float "
			   "*weight; const float *expected; } rms_norm_f32_case;\n\n");

	int sizes[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	int nsizes = sizeof(sizes) / sizeof(sizes[0]);
	float eps = 1e-5f;
	int saved_n[50];

	for (int ci = 0; ci < 50; ci++) {
		int nz = sizes[rand() % nsizes];
		saved_n[ci] = nz;
		float *x = malloc(nz * sizeof(float));
		float *w = malloc(nz * sizeof(float));
		float *e = malloc(nz * sizeof(float));

		float ss = 0.0f;
		for (int j = 0; j < nz; j++) {
			x[j] = (frand() - 0.5f) * 4.0f;
			w[j] = (frand() - 0.5f) * 2.0f + 1.0f;
			ss += x[j] * x[j];
		}
		float inv_rms = 1.0f / sqrtf(ss / (float)nz + eps);
		for (int j = 0; j < nz; j++)
			e[j] = x[j] * inv_rms * w[j];

		fprintf(f, "static float _rms_x_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(x[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _rms_w_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(w[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _rms_e_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(x);
		free(w);
		free(e);
	}

	fprintf(f, "\nstatic const rms_norm_f32_case rms_norm_f32_cases[] = {\n");
	for (int ci = 0; ci < 50; ci++)
		fprintf(f, "    { %d, %.9gf, _rms_x_%d, _rms_w_%d, _rms_e_%d },\n",
				saved_n[ci], eps, ci, ci, ci);
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  rms_norm_f32_golden.h (50 cases)\n");
}

static void gen_vector_swiglu(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/swiglu_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n; const float *gate; const float *up; "
			   "const float *expected; } swiglu_f32_case;\n\n");

	int sizes[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	int nsizes = sizeof(sizes) / sizeof(sizes[0]);
	int saved_n[50];

	for (int ci = 0; ci < 50; ci++) {
		int nz = sizes[rand() % nsizes];
		saved_n[ci] = nz;
		float *gate = malloc(nz * sizeof(float));
		float *up = malloc(nz * sizeof(float));
		float *e = malloc(nz * sizeof(float));
		for (int j = 0; j < nz; j++) {
			gate[j] = (frand() - 0.5f) * 16.0f;
			up[j] = (frand() - 0.5f) * 4.0f;
			e[j] = siluf(gate[j]) * up[j];
		}

		fprintf(f, "static float _swiglu_g_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(gate[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _swiglu_u_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(up[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _swiglu_e_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(gate);
		free(up);
		free(e);
	}

	fprintf(f, "\nstatic const swiglu_f32_case swiglu_f32_cases[] = {\n");
	for (int ci = 0; ci < 50; ci++)
		fprintf(f, "    { %d, _swiglu_g_%d, _swiglu_u_%d, _swiglu_e_%d },\n",
				saved_n[ci], ci, ci, ci);
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  swiglu_f32_golden.h (50 cases)\n");
}

static void gen_vector_softmax(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/softmax_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n; float *input; float *expected; "
			   "} softmax_f32_case;\n\n");

	int sizes[] = { 4, 8, 16, 32, 64, 128 };
	int nsizes = sizeof(sizes) / sizeof(sizes[0]);
	int saved_n[50];

	for (int ci = 0; ci < 50; ci++) {
		int nz = sizes[rand() % nsizes];
		saved_n[ci] = nz;
		float *x = malloc(nz * sizeof(float));
		float *e = malloc(nz * sizeof(float));
		float max = -1e30f;
		for (int j = 0; j < nz; j++) {
			x[j] = (frand() - 0.5f) * 20.0f;
			if (x[j] > max)
				max = x[j];
		}
		float sum = 0.0f;
		for (int j = 0; j < nz; j++) {
			e[j] = expf(x[j] - max);
			sum += e[j];
		}
		for (int j = 0; j < nz; j++)
			e[j] /= sum;

		fprintf(f, "static float _softmax_x_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(x[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _softmax_e_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(x);
		free(e);
	}

	fprintf(f, "\nstatic const softmax_f32_case softmax_f32_cases[] = {\n");
	for (int ci = 0; ci < 50; ci++)
		fprintf(f, "    { %d, _softmax_x_%d, _softmax_e_%d },\n", saved_n[ci],
				ci, ci);
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  softmax_f32_golden.h (50 cases)\n");
}

static void gen_vector_rotary(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/rotary_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n_heads; int half; const float *x; "
			   "const float *cosv; const float *sinv; const float *expected; "
			   "} rotary_f32_case;\n\n");

	int halves[] = { 1, 7, 8, 9, 31, 32, 33, 64 };
	int nhalves = sizeof(halves) / sizeof(halves[0]);
	int heads[] = { 1, 3, 8, 64 };
	int nheads = sizeof(heads) / sizeof(heads[0]);

	for (int ci = 0; ci < 50; ci++) {
		int half = halves[ci % nhalves];
		int nh = heads[(ci / nhalves) % nheads];
		int nz = nh * 2 * half;

		float *x = malloc(nz * sizeof(float));
		float *c = malloc(half * sizeof(float));
		float *s = malloc(half * sizeof(float));
		float *e = malloc(nz * sizeof(float));
		if (!x || !c || !s || !e) {
			fprintf(stderr, "ERROR: out of memory\n");
			exit(1);
		}

		for (int j = 0; j < nz; j++)
			x[j] = (frand() - 0.5f) * 16.0f;
		for (int j = 0; j < half; j++) {
			c[j] = (frand() - 0.5f) * 2.0f;
			s[j] = (frand() - 0.5f) * 2.0f;
		}
		for (int h = 0; h < nh; h++) {
			for (int j = 0; j < half; j++) {
				int base = h * 2 * half;
				e[base + j] = x[base + j] * c[j] - x[base + half + j] * s[j];
				e[base + half + j] =
					x[base + j] * s[j] + x[base + half + j] * c[j];
			}
		}

		fprintf(f, "static float _rotary_x_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(x[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _rotary_c_%d[] = {", ci);
		for (int j = 0; j < half; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(c[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _rotary_s_%d[] = {", ci);
		for (int j = 0; j < half; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(s[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _rotary_e_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(x);
		free(c);
		free(s);
		free(e);
	}

	fprintf(f, "\nstatic const rotary_f32_case rotary_f32_cases[] = {\n");
	for (int ci = 0; ci < 50; ci++) {
		int half = halves[ci % nhalves];
		int nh = heads[(ci / nhalves) % nheads];
		fprintf(f,
				"    { %d, %d, _rotary_x_%d, _rotary_c_%d, _rotary_s_%d, "
				"_rotary_e_%d },\n",
				nh, half, ci, ci, ci, ci);
	}
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  rotary_f32_golden.h (50 cases)\n");
}

static void gen_vector_rows_dot(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/rows_dot_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n_rows; int row_len; int row_stride; "
			   "float scale; const float *q; const float *x; const float "
			   "*expected; } rows_dot_f32_case;\n\n");

	int nrows_set[] = { 0, 1, 7, 8, 64, 256 };
	int nnrows = sizeof(nrows_set) / sizeof(nrows_set[0]);
	int lens[] = { 1, 7, 8, 9, 63, 64 };
	int nlens = sizeof(lens) / sizeof(lens[0]);
	float scales[] = { 1.0f, 0.125f, -2.5f, 0.015625f };
	int nscales = sizeof(scales) / sizeof(scales[0]);
	int stride_extra[] = { 0, 3, 17 };

	for (int ci = 0; ci < 50; ci++) {
		int nr = nrows_set[ci % nnrows];
		int rl = lens[(ci / nnrows) % nlens];
		float scale = scales[(ci / (nnrows * nlens)) % nscales];
		int stride = rl + stride_extra[ci % 3];
		int xlen = stride > 0 ? (nr > 0 ? stride * (nr - 1) + rl : 0) : 0;

		float *q = malloc(rl * sizeof(float));
		float *x = malloc(xlen ? (size_t)xlen * sizeof(float) : 1);
		float *e = malloc(nr * sizeof(float));
		if (!q || !x || !e) {
			fprintf(stderr, "ERROR: out of memory\n");
			exit(1);
		}
		for (int j = 0; j < rl; j++)
			q[j] = (frand() - 0.5f) * 16.0f;
		for (int j = 0; j < xlen; j++)
			x[j] = (frand() - 0.5f) * 16.0f;
		for (int t = 0; t < nr; t++) {
			float s = 0.0f;
			for (int d = 0; d < rl; d++)
				s += q[d] * x[t * stride + d];
			e[t] = s * scale;
		}

		fprintf(f, "static float _rdot_q_%d[] = {", ci);
		for (int j = 0; j < rl; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(q[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _rdot_x_%d[] = {", ci);
		for (int j = 0; j < xlen; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(x[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _rdot_e_%d[] = {", ci);
		for (int j = 0; j < nr; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(q);
		free(x);
		free(e);
	}

	fprintf(f, "\nstatic const rows_dot_f32_case rows_dot_f32_cases[] = {\n");
	for (int ci = 0; ci < 50; ci++) {
		int nr = nrows_set[ci % nnrows];
		int rl = lens[(ci / nnrows) % nlens];
		float scale = scales[(ci / (nnrows * nlens)) % nscales];
		int stride = rl + stride_extra[ci % 3];
		char sbuf[32];
		f32_str(scale, sbuf, sizeof(sbuf));
		fprintf(f,
				"    { %d, %d, %d, %s, _rdot_q_%d, _rdot_x_%d, "
				"_rdot_e_%d },\n",
				nr, rl, stride, sbuf, ci, ci, ci);
	}
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  rows_dot_f32_golden.h (50 cases)\n");
}

static void gen_vector_weighted_sum(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/weighted_sum_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n_rows; int row_len; int row_stride; "
			   "const float *wgt; const float *x; const float *expected; } "
			   "weighted_sum_f32_case;\n\n");

	int nrows_set[] = { 0, 1, 7, 8, 64, 256 };
	int nnrows = sizeof(nrows_set) / sizeof(nrows_set[0]);
	int lens[] = { 1, 7, 8, 9, 63, 64 };
	int nlens = sizeof(lens) / sizeof(lens[0]);

	for (int ci = 0; ci < 50; ci++) {
		int nr = nrows_set[ci % nnrows];
		int rl = lens[(ci / nnrows) % nlens];
		int stride = rl + (ci % 3) * 5;
		int xlen = stride > 0 ? (nr > 0 ? stride * (nr - 1) + rl : 0) : 0;

		float *wgt = malloc(nr * sizeof(float));
		float *x = malloc(xlen ? (size_t)xlen * sizeof(float) : 1);
		float *e = malloc(rl * sizeof(float));
		if (!wgt || !x || !e) {
			fprintf(stderr, "ERROR: out of memory\n");
			exit(1);
		}
		for (int t = 0; t < nr; t++)
			wgt[t] = (frand() - 0.5f) * 4.0f;
		for (int j = 0; j < xlen; j++)
			x[j] = (frand() - 0.5f) * 16.0f;
		for (int d = 0; d < rl; d++) {
			float s = 0.0f;
			for (int t = 0; t < nr; t++)
				s += wgt[t] * x[t * stride + d];
			e[d] = s;
		}

		fprintf(f, "static float _wsum_w_%d[] = {", ci);
		for (int j = 0; j < nr; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(wgt[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _wsum_x_%d[] = {", ci);
		for (int j = 0; j < xlen; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(x[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _wsum_e_%d[] = {", ci);
		for (int j = 0; j < rl; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(wgt);
		free(x);
		free(e);
	}

	fprintf(f, "\nstatic const weighted_sum_f32_case weighted_sum_f32_cases[] "
			   "= {\n");
	for (int ci = 0; ci < 50; ci++) {
		int nr = nrows_set[ci % nnrows];
		int rl = lens[(ci / nnrows) % nlens];
		int stride = rl + (ci % 3) * 5;
		fprintf(f,
				"    { %d, %d, %d, _wsum_w_%d, _wsum_x_%d, _wsum_e_%d "
				"},\n",
				nr, rl, stride, ci, ci, ci);
	}
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  weighted_sum_f32_golden.h (50 cases)\n");
}

static float ref_fp16_to_f32(uint16_t h)
{
	uint32_t sign = (h >> 15) << 31;
	uint32_t exp = (h >> 10) & 0x1F;
	uint32_t mant = h & 0x3FF;
	if (exp == 0) {
		if (mant == 0) {
			uint32_t zero = sign;
			return *(float *)&zero;
		}
		int e = 0;
		while (!(mant & 0x400)) {
			mant <<= 1;
			e++;
		}
		mant &= 0x3FF;
		uint32_t f32 = sign | ((uint32_t)(113 - e) << 23) | (mant << 13);
		return *(float *)&f32;
	}
	if (exp == 31) {
		uint32_t nan_bits = 0x7FC00000;
		uint32_t inf_bits = 0x7F800000;
		return mant ? *(float *)&nan_bits :
					  (sign ? -*(float *)&inf_bits : *(float *)&inf_bits);
	}
	uint32_t f32 = sign | ((exp + 112) << 23) | (mant << 13);
	return *(float *)&f32;
}

static void gen_scalar_fp16_to_f32(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/fp16_to_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { float input; float expected; float abs_tol; "
			   "} fp16_to_f32_case;\n");
	fprintf(f, "static const fp16_to_f32_case fp16_to_f32_cases[] = {\n");

	uint16_t values[] = {
		0x0000, 0x8000, 0x3C00, 0xBC00, 0x7C00, 0xFC00, 0x7E00, 0xFE00, 0x0001,
		0x03FF, 0x0400, 0x3555, 0x3BFF, 0x7BFF, 0xFBFF, 0x4000, 0x4400, 0xC400,
	};
	int nv = sizeof(values) / sizeof(values[0]);
	for (int i = 0; i < nv; i++) {
		float want = ref_fp16_to_f32(values[i]);
		char inbuf[32], wantbuf[32];
		f32_str((float)(int)values[i], inbuf, sizeof(inbuf));
		f32_str(want, wantbuf, sizeof(wantbuf));
		fprintf(f, "    { %s, %s, 1e-7 },\n", inbuf, wantbuf);
	}
	for (int i = 0; i < 100; i++) {
		uint16_t u = (uint16_t)(rand() & 0xFFFF);
		float want = ref_fp16_to_f32(u);
		char inbuf[32], wantbuf[32];
		f32_str((float)(int)u, inbuf, sizeof(inbuf));
		f32_str(want, wantbuf, sizeof(wantbuf));
		fprintf(f, "    { %s, %s, 1e-7 },\n", inbuf, wantbuf);
	}
	fprintf(f, "    { 0, 0, -1.0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  fp16_to_f32_golden.h (%d cases)\n", nv + 100);
}

static void gen_vector_q8_dequant(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/q8_dequant_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n; const uint8_t *src; const float "
			   "*expected; } q8_dequant_f32_case;\n\n");

	int lens[] = { 32, 64, 128, 256 };
	int nlens = sizeof(lens) / sizeof(lens[0]);
	int saved_n[30];

	for (int ci = 0; ci < 30; ci++) {
		int nz = lens[rand() % nlens];
		saved_n[ci] = nz;
		int blocks = nz / 32;
		size_t nbytes = (size_t)blocks * 34;
		uint8_t *src = malloc(nbytes);
		float *e = malloc((size_t)nz * sizeof(float));
		if (!src || !e) {
			fprintf(stderr, "ERROR: out of memory\n");
			exit(1);
		}
		for (int b = 0; b < blocks; b++) {
			uint16_t h;
			float scale;
			do {
				h = (uint16_t)(rand() & 0xFFFF);
				scale = ref_fp16_to_f32(h);
			} while (!isfinite(scale));
			src[b * 34] = (uint8_t)(h & 0xFF);
			src[b * 34 + 1] = (uint8_t)(h >> 8);
			for (int j = 0; j < 32; j++) {
				int8_t qv = (int8_t)(rand() % 256);
				src[b * 34 + 2 + j] = (uint8_t)qv;
				e[b * 32 + j] = (float)qv * scale;
			}
		}

		fprintf(f, "static const uint8_t _qd_src_%d[] = {", ci);
		for (size_t j = 0; j < nbytes; j++) {
			if (j % 20 == 0)
				fprintf(f, "\n    ");
			fprintf(f, "0x%02X, ", src[j]);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _qd_e_%d[] = {", ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(src);
		free(e);
	}

	fprintf(f, "\nstatic const q8_dequant_f32_case q8_dequant_f32_cases[] = "
			   "{\n");
	for (int ci = 0; ci < 30; ci++) {
		fprintf(f, "    { %d, _qd_src_%d, _qd_e_%d },\n", saved_n[ci], ci, ci);
	}
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  q8_dequant_f32_golden.h (30 cases)\n");
}

static void gen_vector_q8_matvec(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/q8_matvec_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n_rows; int cols; const uint8_t *w; "
			   "const float *x; const float *expected; float abs_tol; } "
			   "q8_matvec_f32_case;\n\n");

	int lens[] = { 32, 64, 128 };
	int nlens = sizeof(lens) / sizeof(lens[0]);
	int nrows_set[] = { 0, 1, 4, 16 };
	int nnrows = sizeof(nrows_set) / sizeof(nrows_set[0]);

	int ci = 0;
	for (int li = 0; li < nlens; li++) {
		for (int ri = 0; ri < nnrows; ri++) {
			int cols = lens[li];
			int nr = nrows_set[ri];
			int blocks = cols / 32;
			size_t row_bytes = (size_t)blocks * 34;
			size_t wbytes = (size_t)nr * row_bytes;

			uint8_t *w = malloc(wbytes ? wbytes : 1);
			float *x = malloc((size_t)cols * sizeof(float));
			float *e = malloc((size_t)(nr > 0 ? nr : 1) * sizeof(float));
			if (!w || !x || !e) {
				fprintf(stderr, "ERROR: out of memory\n");
				exit(1);
			}

			for (int r = 0; r < nr; r++) {
				for (int b = 0; b < blocks; b++) {
					uint8_t *blk = w + (size_t)r * row_bytes + (size_t)b * 34;
					uint16_t h;
					float scale;
					do {
						h = (uint16_t)(rand() & 0xFFFF);
						scale = ref_fp16_to_f32(h);
					} while (!isfinite(scale));
					blk[0] = (uint8_t)(h & 0xFF);
					blk[1] = (uint8_t)(h >> 8);
					for (int j = 0; j < 32; j++)
						blk[2 + j] = (uint8_t)(rand() % 256);
				}
			}
			for (int j = 0; j < cols; j++)
				x[j] = (frand() - 0.5f) * 16.0f;

			for (int t = 0; t < nr; t++) {
				float s = 0.0f;
				for (int b = 0; b < blocks; b++) {
					const uint8_t *blk =
						w + (size_t)t * row_bytes + (size_t)b * 34;
					uint16_t h = (uint16_t)blk[0] | ((uint16_t)blk[1] << 8);
					float d = ref_fp16_to_f32(h);
					const int8_t *q = (const int8_t *)(blk + 2);
					for (int j = 0; j < 32; j++)
						s += (float)q[j] * d * x[b * 32 + j];
				}
				e[t] = s;
			}

			fprintf(f, "static const uint8_t _qm_w_%d[] = {", ci);
			for (size_t j = 0; j < wbytes; j++) {
				if (j % 20 == 0)
					fprintf(f, "\n    ");
				fprintf(f, "0x%02X, ", w[j]);
			}
			fprintf(f, "\n};\n");

			fprintf(f, "static float _qm_x_%d[] = {", ci);
			for (int j = 0; j < cols; j++) {
				if (j % 6 == 0)
					fprintf(f, "\n    ");
				char buf[32];
				f32_str(x[j], buf, sizeof(buf));
				fprintf(f, "%s, ", buf);
			}
			fprintf(f, "\n};\n");

			fprintf(f, "static float _qm_e_%d[] = {", ci);
			for (int j = 0; j < nr; j++) {
				if (j % 6 == 0)
					fprintf(f, "\n    ");
				char buf[32];
				f32_str(e[j], buf, sizeof(buf));
				fprintf(f, "%s, ", buf);
			}
			fprintf(f, "\n};\n");

			free(w);
			free(x);
			free(e);
			ci++;
		}
	}

	fprintf(f, "\nstatic const q8_matvec_f32_case q8_matvec_f32_cases[] = "
			   "{\n");
	for (int i = 0; i < ci; i++) {
		int cols = lens[(i / nnrows) % nlens];
		int nr = nrows_set[i % nnrows];
		fprintf(f, "    { %d, %d, _qm_w_%d, _qm_x_%d, _qm_e_%d, 1e-4f },\n", nr,
				cols, i, i, i);
	}
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  q8_matvec_f32_golden.h (%d cases)\n", ci);
}

static const float kvalues_mxfp4[16] = { 0.0f,	0.5f,  1.0f,  1.5f,
										 2.0f,	3.0f,  4.0f,  6.0f,
										 -0.0f, -0.5f, -1.0f, -1.5f,
										 -2.0f, -3.0f, -4.0f, -6.0f };

static float mxfp4_scale_f(uint8_t scale)
{
	if (scale == 0)
		return 0.0f;
	uint32_t u = (uint32_t)scale << 23;
	return *(float *)&u;
}

static void gen_vector_mxfp4_matvec(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/mxfp4_matvec_f32_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f, "typedef struct { int n_rows; int cols; const uint8_t *w; "
			   "const float *x; const float *bias; const float *expected; "
			   "float abs_tol; } mxfp4_matvec_f32_case;\n\n");

	int lens[] = { 32, 128, 2880 };
	int nlens = sizeof(lens) / sizeof(lens[0]);
	int nrows_set[] = { 0, 1, 4, 16 };
	int nnrows = sizeof(nrows_set) / sizeof(nrows_set[0]);

	int ci = 0;
	for (int li = 0; li < nlens; li++) {
		for (int ri = 0; ri < nnrows; ri++) {
			int cols = lens[li];
			int nr = nrows_set[ri];
			int blocks = cols / 32;
			size_t row_bytes = (size_t)blocks * 17;
			size_t wbytes = (size_t)nr * row_bytes;

			uint8_t *w = malloc(wbytes ? wbytes : 1);
			float *x = malloc(cols * sizeof(float));
			float *bias = malloc(nr * sizeof(float));
			float *e = malloc(nr * sizeof(float));
			if (!w || !x || !bias || !e) {
				fprintf(stderr, "ERROR: out of memory\n");
				exit(1);
			}

			/* Scales stay in [64, 190] so products stay in float range;
			 * zero-scale blocks appear in the repeated cases. */
			int rep = ci % 2;
			for (int r = 0; r < nr; r++) {
				for (int b = 0; b < blocks; b++) {
					uint8_t *blk = w + (size_t)r * row_bytes + (size_t)b * 17;
					int force_zero = (b % 5 == 4) && (rep == 1);
					blk[0] = force_zero ? 0 : (uint8_t)(64 + rand() % 127);
					for (int j = 0; j < 16; j++) {
						int lo = rand() % 16;
						int hi = rand() % 16;
						blk[1 + j] = (uint8_t)((hi << 4) | lo);
					}
				}
			}
			for (int j = 0; j < cols; j++)
				x[j] = (frand() - 0.5f) * 16.0f;
			for (int t = 0; t < nr; t++)
				bias[t] = (frand() - 0.5f) * 2.0f;

			/* Scalar reference: mirrors gguf_dequant.c split order:
			 * block stores 16 nibble bytes; low nibble of byte j
			 * is element j (cols 0-15), high nibble is element j+16
			 * (cols 16-31). */
			for (int t = 0; t < nr; t++) {
				float s = 0.0f;
				for (int b = 0; b < blocks; b++) {
					const uint8_t *blk =
						w + (size_t)t * row_bytes + (size_t)b * 17;
					float sf = mxfp4_scale_f(blk[0]);
					for (int j = 0; j < 16; j++) {
						uint8_t nb = blk[1 + j];
						int d = b * 32;
						s += kvalues_mxfp4[nb & 0x0F] * sf * x[d + j];
						s += kvalues_mxfp4[nb >> 4] * sf * x[d + j + 16];
					}
				}
				e[t] = s + bias[t];
			}

			fprintf(f, "static const uint8_t _mxw_%d[] = {", ci);
			for (size_t j = 0; j < wbytes; j++) {
				if (j % 20 == 0)
					fprintf(f, "\n    ");
				fprintf(f, "0x%02X, ", w[j]);
			}
			fprintf(f, "\n};\n");

			fprintf(f, "static float _mxx_%d[] = {", ci);
			for (int j = 0; j < cols; j++) {
				if (j % 6 == 0)
					fprintf(f, "\n    ");
				char buf[32];
				f32_str(x[j], buf, sizeof(buf));
				fprintf(f, "%s, ", buf);
			}
			fprintf(f, "\n};\n");

			fprintf(f, "static float _mxb_%d[] = {", ci);
			for (int j = 0; j < nr; j++) {
				if (j % 6 == 0)
					fprintf(f, "\n    ");
				char buf[32];
				f32_str(bias[j], buf, sizeof(buf));
				fprintf(f, "%s, ", buf);
			}
			fprintf(f, "\n};\n");

			fprintf(f, "static float _mxe_%d[] = {", ci);
			for (int j = 0; j < nr; j++) {
				if (j % 6 == 0)
					fprintf(f, "\n    ");
				char buf[32];
				f32_str(e[j], buf, sizeof(buf));
				fprintf(f, "%s, ", buf);
			}
			fprintf(f, "\n};\n");

			free(w);
			free(x);
			free(bias);
			free(e);
			ci++;
		}
	}

	fprintf(f,
			"\nstatic const mxfp4_matvec_f32_case mxfp4_matvec_f32_cases[] = "
			"{\n");
	for (int i = 0; i < ci; i++) {
		int cols = lens[(i / nnrows) % nlens];
		int nr = nrows_set[i % nnrows];
		fprintf(f,
				"    { %d, %d, _mxw_%d, _mxx_%d, _mxb_%d, _mxe_%d, "
				"1e-4f },\n",
				nr, cols, i, i, i, i);
	}
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  mxfp4_matvec_f32_golden.h (%d cases)\n", ci);
}

static void gen_harness_self_test(void)
{
	char path[256];
	snprintf(path, sizeof(path), "%s/harness_self_test_golden.h", OUTPUT_DIR);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}
	fprintf(f, "typedef struct { float input; float expected; float abs_tol; "
			   "int should_pass; } harness_self_case;\n");
	fprintf(f, "static const harness_self_case harness_self_cases[] = {\n");
	fprintf(f, "    { 4.0f, 2.0f, 1e-4f, 1 },\n");
	fprintf(f, "    { 9.0f, 2.0f, 1e-4f, 0 },\n");
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  harness_self_test_golden.h\n");
}

static void gen_vector_unary(const char *name, float (*fn)(float))
{
	char path[256];
	snprintf(path, sizeof(path), "%s/%s_f32_golden.h", OUTPUT_DIR, name);
	FILE *f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "ERROR: cannot open %s\n", path);
		exit(1);
	}

	fprintf(f,
			"typedef struct { int n; const float *x; const float *expected; "
			"} %s_f32_case;\n\n",
			name);

	int sizes[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	int nsizes = sizeof(sizes) / sizeof(sizes[0]);
	int saved_n[50];

	for (int ci = 0; ci < 50; ci++) {
		int nz = sizes[rand() % nsizes];
		saved_n[ci] = nz;
		float *x = malloc(nz * sizeof(float));
		float *e = malloc(nz * sizeof(float));
		for (int j = 0; j < nz; j++) {
			x[j] = (frand() - 0.5f) * 32.0f;
			e[j] = fn(x[j]);
		}

		fprintf(f, "static float _%s_x_%d[] = {", name, ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(x[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		fprintf(f, "static float _%s_e_%d[] = {", name, ci);
		for (int j = 0; j < nz; j++) {
			if (j % 6 == 0)
				fprintf(f, "\n    ");
			char buf[32];
			f32_str(e[j], buf, sizeof(buf));
			fprintf(f, "%s, ", buf);
		}
		fprintf(f, "\n};\n");

		free(x);
		free(e);
	}

	fprintf(f, "\nstatic const %s_f32_case %s_f32_cases[] = {\n", name, name);
	for (int ci = 0; ci < 50; ci++)
		fprintf(f, "    { %d, _%s_x_%d, _%s_e_%d },\n", saved_n[ci], name, ci,
				name, ci);
	fprintf(f, "    { 0 },\n");
	fprintf(f, "};\n");
	fclose(f);
	printf("  %s_f32_golden.h (50 cases)\n", name);
}

static int wanted(const char **names, int n_names, const char *fn)
{
	for (int i = 0; i < n_names; i++)
		if (strcmp(names[i], fn) == 0)
			return 1;
	return 0;
}

int main(int argc, char **argv)
{
	srand(42);

	const char **only = (const char **)(argv + 1);
	int n_only = argc - 1;

	printf("Generating golden headers...\n\n");

	printf("Scalar:\n");
	if (!n_only || wanted(only, n_only, "sqrt"))
		gen_sqrt();
	if (!n_only || wanted(only, n_only, "exp"))
		gen_exp();
	if (!n_only || wanted(only, n_only, "log"))
		gen_log();
	if (!n_only || wanted(only, n_only, "sin"))
		gen_sin();
	if (!n_only || wanted(only, n_only, "cos"))
		gen_cos();
	if (!n_only || wanted(only, n_only, "tanh"))
		gen_tanh();
	if (!n_only || wanted(only, n_only, "sigmoid"))
		gen_sigmoid();
	if (!n_only || wanted(only, n_only, "silu"))
		gen_silu();
	if (!n_only || wanted(only, n_only, "fp16_to_f32"))
		gen_scalar_fp16_to_f32();

	printf("\nVector:\n");
	if (!n_only || wanted(only, n_only, "silu_f32"))
		gen_vector_silu_f32();
	if (!n_only || wanted(only, n_only, "rms_norm"))
		gen_vector_rms_norm();
	if (!n_only || wanted(only, n_only, "swiglu"))
		gen_vector_swiglu();
	if (!n_only || wanted(only, n_only, "softmax"))
		gen_vector_softmax();
	if (!n_only || wanted(only, n_only, "exp_f32"))
		gen_vector_unary("exp", expf);
	if (!n_only || wanted(only, n_only, "log_f32"))
		gen_vector_unary("log", logf);
	if (!n_only || wanted(only, n_only, "sin_f32"))
		gen_vector_unary("sin", sinf);
	if (!n_only || wanted(only, n_only, "cos_f32"))
		gen_vector_unary("cos", cosf);
	if (!n_only || wanted(only, n_only, "rotary"))
		gen_vector_rotary();
	if (!n_only || wanted(only, n_only, "rows_dot"))
		gen_vector_rows_dot();
	if (!n_only || wanted(only, n_only, "weighted_sum"))
		gen_vector_weighted_sum();
	if (!n_only || wanted(only, n_only, "q8_dequant"))
		gen_vector_q8_dequant();
	if (!n_only || wanted(only, n_only, "q8_matvec"))
		gen_vector_q8_matvec();
	if (!n_only || wanted(only, n_only, "mxfp4_matvec"))
		gen_vector_mxfp4_matvec();
	printf("\nHarness self-test:\n");
	if (!n_only || wanted(only, n_only, "harness_self"))
		gen_harness_self_test();

	printf("\nDone.\n");
	return 0;
}
