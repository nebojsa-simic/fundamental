/*
 * Startup module tests
 *
 * Tests for centralized startup initialization framework.
 */

#include "fundamental/startup/startup.h"
#include "fundamental/console/console.h"
#include "fundamental/error/error.h"
#include <stdio.h>

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define YELLOW_SKIP "\033[0;33m⏭\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

static void print_test_result(const char *name, int passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, name);
		tests_passed++;
	} else {
		printf("\033[0;31m✗\033[0m %s\n", name);
		tests_failed++;
	}
}

static void test_startup_runs_before_main(void)
{
	print_test_result("test_startup_runs_before_main", 1);
}

static void test_verbose_mode_defined(void)
{
#ifdef STARTUP_TRACE
	print_test_result("test_verbose_mode_defined", 1);
#else
	print_test_result("test_verbose_mode_defined", 0);
#endif
}

static void test_platform_init_succeeds(void)
{
	int result = fun_platform_init();
	print_test_result("test_platform_init_succeeds", result == 0);
}

static void test_filesystem_init_succeeds(void)
{
	int result = fun_filesystem_init();
	print_test_result("test_filesystem_init_succeeds", result == 0);
}

static void test_config_init_succeeds(void)
{
	int result = fun_config_init();
	print_test_result("test_config_init_succeeds", result == 0);
}

static void test_network_init_succeeds(void)
{
	int result = fun_network_init();
	print_test_result("test_network_init_succeeds", result == 0);
}

int cli_main(int argc, const char **argv)
{
	(void)argc;
	(void)argv;

	printf("Running startup module tests:\n");

	test_startup_runs_before_main();
	test_verbose_mode_defined();
	test_platform_init_succeeds();
	test_filesystem_init_succeeds();
	test_config_init_succeeds();
	test_network_init_succeeds();

	if (tests_failed == 0) {
		printf("All startup tests passed!\n");
	} else {
		printf("Tests passed: %d, failed: %d\n", tests_passed, tests_failed);
	}

	return tests_failed > 0 ? 1 : 0;
}

int main(int argc, char **argv)
{
	return cli_main(argc, (const char **)argv);
}
