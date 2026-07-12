#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_harness.h"

#include "test_data/silu_f32_golden.h"
#include "test_data/rms_norm_f32_golden.h"
#include "test_data/swiglu_f32_golden.h"
#include "test_data/softmax_f32_golden.h"

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

    return total;
}
