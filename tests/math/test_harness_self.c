#include "fundamental/console/console.h"
#include "test_harness.h"

#include "test_data/harness_self_test_golden.h"

void math_test_noop(void)
{
}

TestCount test_harness_self(void)
{
	TestCount tc = math_test_count_init();

	fun_console_write_line("");
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
			fun_console_write("    harness self-test: case ");
			char _buf[32];
			fun_string_from_int(i, 10, _buf, sizeof(_buf));
			fun_console_write(_buf);
			fun_console_write(passed ? " passed (expected " :
									   " failed (expected ");
			fun_console_write_line(should_pass ? "pass)" : "fail)");
			tc.failed++;
		} else {
			tc.passed++;
		}
	}

	fun_console_write("    harness self-test: ");
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

	if (tc.failed > 0)
		return tc;

	tc.passed = 1;
	tc.failed = 0;
	return tc;
}
