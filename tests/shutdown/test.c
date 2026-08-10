#include "fundamental/shutdown/shutdown.h"
#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"
#define YELLOW_SKIP "\033[0;33m\342\217\255\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

static void print_test_result(const char *name, int passed)
{
	if (passed) {
		fun_console_write(GREEN_CHECK);
		fun_console_write(" ");
		fun_console_write_line(name);
		tests_passed++;
	} else {
		fun_console_write("\033[0;31m\342\234\227\033[0m");
		fun_console_write(" ");
		fun_console_write_line(name);
		tests_failed++;
	}
}

static void print_test_skip(const char *name)
{
	fun_console_write(YELLOW_SKIP);
	fun_console_write(" ");
	fun_console_write_line(name);
}

static void test_framework_compiles_and_links(void)
{
	print_test_result("test_framework_compiles_and_links", 1);
}

static void test_phase_constants_defined(void)
{
	int ok = (SHUTDOWN_PHASE_PLATFORM == 1 && SHUTDOWN_PHASE_APP == 99);
	print_test_result("test_phase_constants_defined", ok);
}

static void test_signal_handlers(void)
{
	print_test_skip("test_signal_handlers (would intercept Ctrl+C)");
}

static void test_idempotency(void)
{
	print_test_skip("test_idempotency (would exit process)");
}

int main(void)
{
	fun_console_write_line("Running shutdown module tests:");

	test_framework_compiles_and_links();
	test_phase_constants_defined();
	test_signal_handlers();
	test_idempotency();

	if (tests_failed == 0) {
		fun_console_write_line("All shutdown tests passed!");
	} else {
		fun_console_write_line("Tests completed.");
	}

	return tests_failed > 0 ? 1 : 0;
}
