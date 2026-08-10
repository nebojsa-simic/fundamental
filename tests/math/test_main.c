#include "fundamental/console/console.h"
#include "test_harness.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define RED_CROSS "\033[0;31m\u2717\033[0m"

TestCount test_scalar_accuracy(void);
TestCount test_vector_accuracy(void);
TestCount test_edge_cases(void);
TestCount test_consistency(void);
TestCount test_performance(void);
TestCount test_harness_self(void);

static void run_suite(const char *name, TestCount (*fn)(void), TestCount *total)
{
	fun_console_write("  ");
	fun_console_write(name);
	fun_console_write(" ... ");
	TestCount tc = fn();
	if (math_test_count_ok(tc)) {
		fun_console_write(GREEN_CHECK);
		fun_console_write(" passed (");
		char _buf[32];
		fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write_line(")");
	} else {
		fun_console_write(RED_CROSS);
		char _buf[32];
		fun_string_from_int(tc.failed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write(" failed, ");
		fun_string_from_int(tc.passed, 10, _buf, sizeof(_buf));
		fun_console_write(_buf);
		fun_console_write_line(" passed");
	}
	math_test_count_merge(total, tc);
}

extern void math_test_noop(void);

int main(void)
{
	TestCount total = math_test_count_init();

	fun_console_write_line("Math module tests:");

	fun_console_write("  Stubs ... ");
	math_test_noop();
	fun_console_write_line(GREEN_CHECK);

	run_suite("scalar accuracy", test_scalar_accuracy, &total);
	run_suite("vector accuracy", test_vector_accuracy, &total);
	run_suite("edge cases", test_edge_cases, &total);
	run_suite("consistency", test_consistency, &total);
	run_suite("performance", test_performance, &total);
	run_suite("harness self-test", test_harness_self, &total);

	fun_console_write_line("");
	fun_console_write("Summary: ");
	char _buf[32];
	fun_string_from_int(total.passed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write(" passed, ");
	fun_string_from_int(total.failed, 10, _buf, sizeof(_buf));
	fun_console_write(_buf);
	fun_console_write_line(" failed");

	if (total.failed > 0) {
		fun_console_write_line("FAIL");
		return 1;
	}

	fun_console_write_line("All tests passed!");
	return 0;
}
