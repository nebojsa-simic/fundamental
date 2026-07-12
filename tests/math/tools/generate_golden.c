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

    fprintf(f, "typedef struct { float input; float expected; float abs_tol; "
               "} %s_case;\n",
            name);
    fprintf(f, "static const %s_case %s_cases[] = {\n", name, name);

    char ia[32], ea[32];
    for (int i = 0; i < n; i++) {
        f32_str(inputs[i], ia, sizeof(ia));
        f32_str(expected[i], ea, sizeof(ea));
        fprintf(f, "    { %s, %s, %.9gf },\n", ia, ea, abs_tols[i]);
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
        0.0f,  1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  8.0f,
        9.0f,  10.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f,
        81.0f, 100.0f, 0.25f, 0.01f, 0.000001f, 10000.0f,
        1000000.f, 3.14159265f, 0.5f, 7.0f, 0.0625f,
        121.0f, 144.0f, 1.5f, 6.25f,
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
    float b[] = { 0.0f, 1.0f, -1.0f, 2.0f, -2.0f, 5.0f, -5.0f,
                  10.0f, -10.0f, 20.0f, -20.0f, 0.5f, -0.5f };
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
    float b[] = { 0.001f, 0.01f, 0.1f, 0.5f, 1.0f, 2.0f,
                  2.71828183f, 10.0f, 100.0f, 1e6f };
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
    float b[] = { 0.0f, (float)(M_PI / 6), (float)(M_PI / 4),
                  (float)(M_PI / 3), (float)(M_PI / 2), (float)M_PI,
                  (float)(3 * M_PI / 2), (float)(2 * M_PI),
                  (float)(-M_PI / 2), (float)(-M_PI), 1.0f, -1.0f };
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
    float b[] = { 0.0f, (float)(M_PI / 6), (float)(M_PI / 4),
                  (float)(M_PI / 3), (float)(M_PI / 2), (float)M_PI,
                  (float)(3 * M_PI / 2), (float)(2 * M_PI),
                  (float)(-M_PI / 2), (float)(-M_PI) };
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
                  0.5f,  1.0f,  2.0f,  3.0f,  4.0f };
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
    float b[] = { -8.0f, -6.0f, -4.0f, -3.0f, -2.0f, -1.0f, -0.5f,
                  0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 6.0f,
                  8.0f };
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
    float b[] = { -8.0f, -6.0f, -4.0f, -3.0f, -2.0f, -1.0f, -0.5f,
                  0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 6.0f,
                  8.0f };
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
    snprintf(path, sizeof(path), "%s/rms_norm_f32_golden.h",
             OUTPUT_DIR);
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
    snprintf(path, sizeof(path), "%s/swiglu_f32_golden.h",
             OUTPUT_DIR);
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
    snprintf(path, sizeof(path), "%s/softmax_f32_golden.h",
             OUTPUT_DIR);
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
        fprintf(f, "    { %d, _softmax_x_%d, _softmax_e_%d },\n",
                saved_n[ci], ci, ci);
    fprintf(f, "    { 0 },\n");
    fprintf(f, "};\n");
    fclose(f);
    printf("  softmax_f32_golden.h (50 cases)\n");
}

static void gen_harness_self_test(void)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/harness_self_test_golden.h",
             OUTPUT_DIR);
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

int main(void)
{
    srand(42);

    printf("Generating golden headers...\n\n");

    printf("Scalar:\n");
    gen_sqrt();
    gen_exp();
    gen_log();
    gen_sin();
    gen_cos();
    gen_tanh();
    gen_sigmoid();
    gen_silu();

    printf("\nVector:\n");
    gen_vector_silu_f32();
    gen_vector_rms_norm();
    gen_vector_swiglu();
    gen_vector_softmax();

    printf("\nHarness self-test:\n");
    gen_harness_self_test();

    printf("\nDone.\n");
    return 0;
}
