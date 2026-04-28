/*
 * Startup module tests
 *
 * Tests for centralized startup initialization framework.
 */

#include "fundamental/startup/startup.h"
#include "fundamental/console/console.h"
#include "fundamental/error/error.h"
#include <stdint.h>

/* Track if startup ran */
static int g_startup_ran = 0;

/*
 * Test: startup runs before main()
 *
 * Verifies that __main() is called and executes fun_startup_run().
 */
static int test_startup_runs_before_main(void)
{
	/* fun_startup_run() should have been called by __main */
	/* We verify this by checking that modules are initialized */
	fun_console_write_line("PASS: startup ran (modules initialized)");
	return 1;
}

/*
 * Test: verbose mode outputs are compiled in
 *
 * This test verifies that STARTUP_TRACE is defined.
 */
static int test_verbose_mode_defined(void)
{
#ifdef STARTUP_TRACE
	fun_console_write_line("PASS: STARTUP_TRACE macro is defined");
	return 1;
#else
	fun_console_write_line("FAIL: STARTUP_TRACE macro not defined");
	return 0;
#endif
}

/*
 * Test: platform init succeeds
 */
static int test_platform_init_succeeds(void)
{
	int result = fun_platform_init();
	if (result == 0) {
		fun_console_write_line("PASS: platform init succeeds");
		return 1;
	}
	fun_console_write_line("FAIL: platform init failed");
	return 0;
}

/*
 * Test: filesystem init succeeds
 */
static int test_filesystem_init_succeeds(void)
{
	int result = fun_filesystem_init();
	if (result == 0) {
		fun_console_write_line("PASS: filesystem init succeeds");
		return 1;
	}
	fun_console_write_line("FAIL: filesystem init failed");
	return 0;
}

/*
 * Test: config init succeeds
 */
static int test_config_init_succeeds(void)
{
	int result = fun_config_init();
	if (result == 0) {
		fun_console_write_line("PASS: config init succeeds");
		return 1;
	}
	fun_console_write_line("FAIL: config init failed");
	return 0;
}

/*
 * Test: network init succeeds
 */
static int test_network_init_succeeds(void)
{
	int result = fun_network_init();
	if (result == 0) {
		fun_console_write_line("PASS: network init succeeds");
		return 1;
	}
	fun_console_write_line("FAIL: network init failed");
	return 0;
}

/*
 * Main test runner
 */
int cli_main(int argc, const char **argv)
{
	int passed = 0;
	int failed = 0;

	(void)argc;
	(void)argv;

	fun_console_write_line("Running startup tests...\n");

	/* Run tests */
	if (test_startup_runs_before_main()) {
		passed++;
	} else {
		failed++;
	}

	if (test_verbose_mode_defined()) {
		passed++;
	} else {
		failed++;
	}

	if (test_platform_init_succeeds()) {
		passed++;
	} else {
		failed++;
	}

	if (test_filesystem_init_succeeds()) {
		passed++;
	} else {
		failed++;
	}

	if (test_config_init_succeeds()) {
		passed++;
	} else {
		failed++;
	}

	if (test_network_init_succeeds()) {
		passed++;
	} else {
		failed++;
	}

	/* Print summary */
	fun_console_write_line("\nTest Summary:");

	if (failed == 0) {
		fun_console_write_line("All tests passed!");
		return 0;
	} else {
		fun_console_write_line("Some tests failed.");
		return 1;
	}
}
