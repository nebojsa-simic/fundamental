#include <stdio.h>
#include "test_harness.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define RED_CROSS "\033[0;31m\u2717\033[0m"

TestCount test_scalar_accuracy(void);
TestCount test_vector_accuracy(void);
TestCount test_edge_cases(void);
TestCount test_consistency(void);
TestCount test_performance(void);
TestCount test_harness_self(void);

static void run_suite(const char *name, TestCount (*fn)(void),
                      TestCount *total)
{
    printf("  %s ... ", name);
    TestCount tc = fn();
    if (math_test_count_ok(tc)) {
        printf("%s passed (%d)\n", GREEN_CHECK, tc.passed);
    } else {
        printf("%s %d failed, %d passed\n", RED_CROSS, tc.failed,
               tc.passed);
    }
    math_test_count_merge(total, tc);
}

extern void math_test_noop(void);

int main(void)
{
    TestCount total = math_test_count_init();

    printf("Math module tests:\n");

    printf("  Stubs ... ");
    math_test_noop();
    printf("%s\n", GREEN_CHECK);

    run_suite("scalar accuracy", test_scalar_accuracy, &total);
    run_suite("vector accuracy", test_vector_accuracy, &total);
    run_suite("edge cases", test_edge_cases, &total);
    run_suite("consistency", test_consistency, &total);
    run_suite("performance", test_performance, &total);
    run_suite("harness self-test", test_harness_self, &total);

    printf("\nSummary: %d passed, %d failed\n", total.passed, total.failed);

    if (total.failed > 0) {
        printf("FAIL\n");
        return 1;
    }

    printf("All tests passed!\n");
    return 0;
}
