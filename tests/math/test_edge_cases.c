#include <math.h>
#include <stdio.h>
#include "test_harness.h"

static int check_int(int condition, TestCount *tc, const char *msg)
{
    if (!condition) {
        printf("\n    FAIL: %s\n", msg);
        tc->failed++;
        return 0;
    }
    tc->passed++;
    return 1;
}

TestCount test_edge_cases(void)
{
    TestCount tc = math_test_count_init();

    printf("\n");

    float nan = _math_test_make_nan();
    float inf = _math_test_make_inf();
    float neg_inf = _math_test_make_neg_inf();
    float neg_zero = _math_test_make_neg_zero();

    /* sqrt edge cases */
    printf("    sqrt: ");
    {
        int start = tc.passed + tc.failed;
        check_int(_math_test_check_same_sign(fun_math_sqrt(0.0f), 0.0f), &tc,
                  "sqrt(0.0) should be +0");
        check_int(_math_test_check_same_sign(fun_math_sqrt(neg_zero),
                                             neg_zero),
                  &tc, "sqrt(-0.0) should be -0");
        check_int(_math_test_float_is_inf(fun_math_sqrt(inf)), &tc,
                  "sqrt(+inf) should be +inf");
        check_int(_math_test_float_is_nan(fun_math_sqrt(-1.0f)), &tc,
                  "sqrt(negative) should be NaN");
        check_int(_math_test_float_is_nan(fun_math_sqrt(nan)), &tc,
                  "sqrt(NaN) should be NaN");
        printf("%d ok\n", (tc.passed + tc.failed) - start);
    }

    /* exp edge cases */
    printf("    exp: ");
    {
        int start = tc.passed + tc.failed;
        check_int(fun_math_exp(0.0f) == 1.0f, &tc, "exp(0) should be 1");
        check_int(_math_test_float_is_inf(fun_math_exp(inf)), &tc,
                  "exp(+inf) should be +inf");
        check_int(fun_math_exp(neg_inf) == 0.0f, &tc,
                  "exp(-inf) should be 0");
        check_int(_math_test_float_is_nan(fun_math_exp(nan)), &tc,
                  "exp(NaN) should be NaN");
        check_int(_math_test_float_is_inf(fun_math_exp(100.0f)), &tc,
                  "exp(100) should be +inf (overflow)");
        check_int(fun_math_exp(-100.0f) == 0.0f, &tc,
                  "exp(-100) should be 0 (underflow)");
        printf("%d ok\n", (tc.passed + tc.failed) - start);
    }

    /* log edge cases */
    printf("    log: ");
    {
        int start = tc.passed + tc.failed;
        check_int(fun_math_log(1.0f) == 0.0f, &tc, "log(1) should be 0");
        check_int(_math_test_float_is_inf(fun_math_log(inf)), &tc,
                  "log(+inf) should be +inf");
        check_int(fun_math_log(0.0f) == neg_inf || _math_test_float_is_inf(
                        fun_math_log(0.0f)) && fun_math_log(0.0f) < 0.0f,
                  &tc,
                  "log(0.0) should be -inf");
        check_int(fun_math_log(neg_zero) == neg_inf || _math_test_float_is_inf(
                        fun_math_log(neg_zero)) && fun_math_log(neg_zero) < 0.0f,
                  &tc,
                  "log(-0.0) should be -inf");
        check_int(_math_test_float_is_nan(fun_math_log(-1.0f)), &tc,
                  "log(negative) should be NaN");
        check_int(_math_test_float_is_nan(fun_math_log(nan)), &tc,
                  "log(NaN) should be NaN");
        printf("%d ok\n", (tc.passed + tc.failed) - start);
    }

    /* sin/cos edge cases */
    printf("    sin/cos: ");
    {
        int start = tc.passed + tc.failed;
        check_int(fun_math_sin(0.0f) == 0.0f, &tc, "sin(0) should be 0");
        check_int(_math_test_float_is_nan(fun_math_sin(nan)), &tc,
                  "sin(NaN) should be NaN");
        check_int(fun_math_cos(0.0f) == 1.0f, &tc, "cos(0) should be 1");
        check_int(_math_test_float_is_nan(fun_math_cos(nan)), &tc,
                  "cos(NaN) should be NaN");
        printf("%d ok\n", (tc.passed + tc.failed) - start);
    }

    /* tanh edge cases */
    printf("    tanh: ");
    {
        int start = tc.passed + tc.failed;
        check_int(fun_math_tanh(0.0f) == 0.0f, &tc, "tanh(0) should be 0");
        check_int(fun_math_tanh(inf) == 1.0f, &tc, "tanh(+inf) should be 1");
        check_int(fun_math_tanh(neg_inf) == -1.0f, &tc,
                  "tanh(-inf) should be -1");
        check_int(_math_test_float_is_nan(fun_math_tanh(nan)), &tc,
                  "tanh(NaN) should be NaN");
        printf("%d ok\n", (tc.passed + tc.failed) - start);
    }

    /* sigmoid edge cases */
    printf("    sigmoid: ");
    {
        int start = tc.passed + tc.failed;
        check_int(fun_math_sigmoid(0.0f) == 0.5f, &tc,
                  "sigmoid(0) should be 0.5");
        float sig_pos = fun_math_sigmoid(20.0f);
        check_int(sig_pos > 0.999f && sig_pos < 1.001f, &tc,
                  "sigmoid(large) should be ~1");
        float sig_neg = fun_math_sigmoid(-20.0f);
        check_int(sig_neg > -0.001f && sig_neg < 0.001f, &tc,
                  "sigmoid(-large) should be ~0");
        check_int(_math_test_float_is_nan(fun_math_sigmoid(nan)), &tc,
                  "sigmoid(NaN) should be NaN");
        printf("%d ok\n", (tc.passed + tc.failed) - start);
    }

    /* zero-length vector calls */
    printf("    zero-length vectors: ");
    {
        int start = tc.passed + tc.failed;
        float buf = 0.0f;
        fun_math_silu_f32(&buf, &buf, 0);
        check_int(1, &tc, "silu_f32(n=0) should return");
        fun_math_rms_norm_f32(&buf, &buf, &buf, 0, 0.0f);
        check_int(1, &tc, "rms_norm_f32(n=0) should return");
        fun_math_swiglu_f32(&buf, &buf, &buf, 0);
        check_int(1, &tc, "swiglu_f32(n=0) should return");
        fun_math_softmax_f32(&buf, 0);
        check_int(1, &tc, "softmax_f32(n=0) should return");
        printf("%d ok\n", (tc.passed + tc.failed) - start);
    }

    return tc;
}
