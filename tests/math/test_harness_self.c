#include <math.h>
#include <stdio.h>
#include "test_harness.h"

#include "test_data/harness_self_test_golden.h"

void math_test_noop(void)
{
}

TestCount test_harness_self(void)
{
    TestCount tc = math_test_count_init();

    printf("\n");
    for (int i = 0;; i++) {
        float input = harness_self_cases[i].input;
        float expected = harness_self_cases[i].expected;
        float abs_tol = harness_self_cases[i].abs_tol;
        int should_pass = harness_self_cases[i].should_pass;

        if (input == 0.0f && expected == 0.0f && abs_tol == 0.0f &&
            should_pass == 0)
            break;

        float got = fun_math_sqrt(input);
        int passed = _math_test_check_float(got, expected, abs_tol, 1e-4f);

        if (passed != should_pass) {
            printf("    harness self-test: case %d %s (expected %s)\n", i,
                   passed ? "passed" : "failed",
                   should_pass ? "pass" : "fail");
            tc.failed++;
        } else {
            tc.passed++;
        }
    }

    printf("    harness self-test: %d/%d", tc.passed, tc.passed + tc.failed);
    if (tc.failed)
        printf(" (%d FAILED)", tc.failed);
    printf("\n");

    if (tc.failed > 0)
        return tc;

    tc.passed = 1;
    tc.failed = 0;
    return tc;
}
