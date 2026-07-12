#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_harness.h"

static __inline__ uint64_t _math_test_rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static void bench_scalar(const char *name, float (*fn)(float), int n,
                          int reps)
{
    uint64_t best = ~0ULL;

    float *x = malloc(n * sizeof(float));
    float *out = malloc(n * sizeof(float));
    if (!x || !out)
        return;

    for (int i = 0; i < n; i++)
        x[i] = (float)(i % 256) * 0.1f - 12.8f;

    for (int r = 0; r < reps; r++) {
        uint64_t t0 = _math_test_rdtsc();
        for (int i = 0; i < n; i++)
            out[i] = fn(x[i]);
        uint64_t t1 = _math_test_rdtsc();
        uint64_t elapsed = t1 - t0;
        if (elapsed < best)
            best = elapsed;
    }

    printf("    %-10s: %8.2f cyc/el (%llu cycles / %d)\n", name,
           (double)best / (double)n, (unsigned long long)best, n);

    free(x);
    free(out);
}

static void bench_vector(const char *name,
                          void (*fn)(const float *, float *, size_t), int n,
                          int reps)
{
    uint64_t best = ~0ULL;

    float *x = malloc(n * sizeof(float));
    float *out = malloc(n * sizeof(float));
    if (!x || !out)
        return;

    for (int i = 0; i < n; i++)
        x[i] = (float)(i % 256) * 0.1f - 12.8f;

    for (int r = 0; r < reps; r++) {
        uint64_t t0 = _math_test_rdtsc();
        fn(x, out, n);
        uint64_t t1 = _math_test_rdtsc();
        uint64_t elapsed = t1 - t0;
        if (elapsed < best)
            best = elapsed;
    }

    printf("    %-10s: %8.2f cyc/el (%llu cycles / %d)\n", name,
           (double)best / (double)n, (unsigned long long)best, n);

    free(x);
    free(out);
}

static void bench_vector_swiglu(const char *name,
                                 void (*fn)(const float *, const float *,
                                             float *, size_t),
                                 int n, int reps)
{
    uint64_t best = ~0ULL;

    float *gate = malloc(n * sizeof(float));
    float *up = malloc(n * sizeof(float));
    float *out = malloc(n * sizeof(float));
    if (!gate || !up || !out)
        goto done;

    for (int i = 0; i < n; i++) {
        gate[i] = (float)(i % 256) * 0.1f - 12.8f;
        up[i] = (float)(i % 128) * 0.05f - 3.2f;
    }

    for (int r = 0; r < reps; r++) {
        uint64_t t0 = _math_test_rdtsc();
        fn(gate, up, out, n);
        uint64_t t1 = _math_test_rdtsc();
        uint64_t elapsed = t1 - t0;
        if (elapsed < best)
            best = elapsed;
    }

    printf("    %-10s: %8.2f cyc/el (%llu cycles / %d)\n", name,
           (double)best / (double)n, (unsigned long long)best, n);

done:
    free(gate);
    free(up);
    free(out);
}

static void bench_vector_rms(const char *name,
                              void (*fn)(const float *, const float *, float *,
                                          size_t, float),
                              int n, int reps, float eps)
{
    uint64_t best = ~0ULL;

    float *x = malloc(n * sizeof(float));
    float *w = malloc(n * sizeof(float));
    float *out = malloc(n * sizeof(float));
    if (!x || !w || !out)
        goto done;

    for (int i = 0; i < n; i++) {
        x[i] = (float)(i % 256) * 0.1f - 12.8f;
        w[i] = 1.0f;
    }

    for (int r = 0; r < reps; r++) {
        uint64_t t0 = _math_test_rdtsc();
        fn(x, w, out, n, eps);
        uint64_t t1 = _math_test_rdtsc();
        uint64_t elapsed = t1 - t0;
        if (elapsed < best)
            best = elapsed;
    }

    printf("    %-10s: %8.2f cyc/el (%llu cycles / %d)\n", name,
           (double)best / (double)n, (unsigned long long)best, n);

done:
    free(x);
    free(w);
    free(out);
}

static void bench_noop(void)
{
    int n = 65536;
    int reps = 100;
    uint64_t best = ~0ULL;

    float *x = malloc(n * sizeof(float));
    float *out = malloc(n * sizeof(float));
    if (!x || !out)
        return;

    for (int i = 0; i < n; i++)
        x[i] = (float)i;

    for (int r = 0; r < reps; r++) {
        uint64_t t0 = _math_test_rdtsc();
        _math_bench_noop(x, out, n);
        uint64_t t1 = _math_test_rdtsc();
        uint64_t elapsed = t1 - t0;
        if (elapsed < best)
            best = elapsed;
    }

    printf("    %-10s: %8.2f cyc/el (loop overhead)\n", "noop",
           (double)best / (double)n);

    free(x);
    free(out);
}

TestCount test_performance(void)
{
    TestCount tc = math_test_count_init();

    printf("\n");
    printf("    Scalar (n=1024, best of 100000):\n");
    bench_scalar("sqrt", fun_math_sqrt, 1024, 100000);
    bench_scalar("exp", fun_math_exp, 1024, 100000);
    bench_scalar("log", fun_math_log, 1024, 100000);
    bench_scalar("sin", fun_math_sin, 1024, 100000);
    bench_scalar("cos", fun_math_cos, 1024, 100000);
    bench_scalar("tanh", fun_math_tanh, 1024, 100000);
    bench_scalar("sigmoid", fun_math_sigmoid, 1024, 100000);
    bench_scalar("silu", fun_math_silu, 1024, 100000);

    printf("\n    Vector (n=65536, best of 10000):\n");
    bench_noop();
    bench_vector("silu_f32", fun_math_silu_f32, 65536, 10000);
    bench_vector_rms("rms_norm", fun_math_rms_norm_f32, 65536, 1000, 1e-5f);
    bench_vector_swiglu("swiglu_f32", fun_math_swiglu_f32, 65536, 1000);

    {
        float *x = malloc(64 * sizeof(float));
        float *xcopy = malloc(64 * sizeof(float));
        if (x && xcopy) {
            bench_vector("softmax", NULL, 0, 0);

            uint64_t best = ~0ULL;
            for (int i = 0; i < 64; i++)
                x[i] = (float)(i * i) * 0.01f;

            for (int r = 0; r < 100000; r++) {
                for (int i = 0; i < 64; i++)
                    xcopy[i] = x[i];
                uint64_t t0 = _math_test_rdtsc();
                fun_math_softmax_f32(xcopy, 64);
                uint64_t t1 = _math_test_rdtsc();
                if (t1 - t0 < best)
                    best = t1 - t0;
            }
            printf("    %-10s: %8.2f cyc/el (%llu cycles / %d)\n",
                   "softmax", (double)best / 64.0,
                   (unsigned long long)best, 64);
        }
        free(x);
        free(xcopy);
    }

    tc.passed = 1;
    return tc;
}
