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
            int ok = _math_test_check_float(simd[i], scalar[i], 1e-4f,
                                             1e-3f);
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
            int ok = _math_test_check_float(simd[i], scalar_ref[i], 5e-4f,
                                             5e-3f);
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

    return total;
}
