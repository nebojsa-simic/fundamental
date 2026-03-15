#include "platform/platform.h"
#include "string/string.h"
#include <stdio.h>
#include <string.h>

#define GREEN_CHECK "\033[0;32m✓\033[0m"
#define RED_CROSS "\033[0;31m✗\033[0m"

static void print_test_result(const char *test_name, int passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, test_name);
	} else {
		printf("%s %s\n", RED_CROSS, test_name);
	}
}

// ============================================================================
// Tests for fun_platform_get
// ============================================================================

static void test_fun_platform_get_fills_struct(void)
{
	Platform p;
	fun_platform_get(&p);

	int passed =
		(p.os != PLATFORM_OS_UNKNOWN && p.arch != PLATFORM_ARCH_UNKNOWN);

	print_test_result("fun_platform_get: fills struct with known values",
					  passed);
}

// ============================================================================
// Tests for fun_platform_os_to_string
// ============================================================================

static void test_fun_platform_os_to_string_windows(void)
{
	char buf[16];
	ErrorResult r =
		fun_platform_os_to_string(PLATFORM_OS_WINDOWS, buf, sizeof(buf));

	int passed = !fun_error_is_error(r) && strcmp(buf, "windows") == 0;

	print_test_result("fun_platform_os_to_string: PLATFORM_OS_WINDOWS", passed);
}

static void test_fun_platform_os_to_string_linux(void)
{
	char buf[16];
	ErrorResult r =
		fun_platform_os_to_string(PLATFORM_OS_LINUX, buf, sizeof(buf));

	int passed = !fun_error_is_error(r) && strcmp(buf, "linux") == 0;

	print_test_result("fun_platform_os_to_string: PLATFORM_OS_LINUX", passed);
}

static void test_fun_platform_os_to_string_darwin(void)
{
	char buf[16];
	ErrorResult r =
		fun_platform_os_to_string(PLATFORM_OS_DARWIN, buf, sizeof(buf));

	int passed = !fun_error_is_error(r) && strcmp(buf, "darwin") == 0;

	print_test_result("fun_platform_os_to_string: PLATFORM_OS_DARWIN", passed);
}

static void test_fun_platform_os_to_string_unknown(void)
{
	char buf[16];
	ErrorResult r =
		fun_platform_os_to_string(PLATFORM_OS_UNKNOWN, buf, sizeof(buf));

	int passed = !fun_error_is_error(r) && strcmp(buf, "unknown") == 0;

	print_test_result("fun_platform_os_to_string: PLATFORM_OS_UNKNOWN", passed);
}

static void test_fun_platform_os_to_string_null(void)
{
	ErrorResult r = fun_platform_os_to_string(PLATFORM_OS_WINDOWS, NULL, 0);

	int passed = fun_error_is_error(r);

	print_test_result("fun_platform_os_to_string: NULL buffer", passed);
}

// ============================================================================
// Tests for fun_platform_arch_to_string
// ============================================================================

static void test_fun_platform_arch_to_string_amd64(void)
{
	char buf[16];
	ErrorResult r =
		fun_platform_arch_to_string(PLATFORM_ARCH_AMD64, buf, sizeof(buf));

	int passed = !fun_error_is_error(r) && strcmp(buf, "amd64") == 0;

	print_test_result("fun_platform_arch_to_string: PLATFORM_ARCH_AMD64",
					  passed);
}

static void test_fun_platform_arch_to_string_arm64(void)
{
	char buf[16];
	ErrorResult r =
		fun_platform_arch_to_string(PLATFORM_ARCH_ARM64, buf, sizeof(buf));

	int passed = !fun_error_is_error(r) && strcmp(buf, "arm64") == 0;

	print_test_result("fun_platform_arch_to_string: PLATFORM_ARCH_ARM64",
					  passed);
}

static void test_fun_platform_arch_to_string_unknown(void)
{
	char buf[16];
	ErrorResult r =
		fun_platform_arch_to_string(PLATFORM_ARCH_UNKNOWN, buf, sizeof(buf));

	int passed = !fun_error_is_error(r) && strcmp(buf, "unknown") == 0;

	print_test_result("fun_platform_arch_to_string: PLATFORM_ARCH_UNKNOWN",
					  passed);
}

static void test_fun_platform_arch_to_string_null(void)
{
	ErrorResult r = fun_platform_arch_to_string(PLATFORM_ARCH_AMD64, NULL, 0);

	int passed = fun_error_is_error(r);

	print_test_result("fun_platform_arch_to_string: NULL buffer", passed);
}

// ============================================================================
// Tests for fun_platform_to_string
// ============================================================================

static void test_fun_platform_to_string_format(void)
{
	Platform p;
	fun_platform_get(&p);

	char buf[32];
	voidResult r = fun_platform_to_string(p, buf, sizeof(buf));

	char os_buf[16];
	char arch_buf[16];
	fun_platform_os_to_string(p.os, os_buf, sizeof(os_buf));
	fun_platform_arch_to_string(p.arch, arch_buf, sizeof(arch_buf));

	char expected[32];
	snprintf(expected, sizeof(expected), "%s-%s", os_buf, arch_buf);

	int passed = !fun_error_is_error(r.error) && strcmp(buf, expected) == 0;

	print_test_result("fun_platform_to_string: correct os-arch format", passed);
}

static void test_fun_platform_to_string_null(void)
{
	Platform p;
	fun_platform_get(&p);

	voidResult r = fun_platform_to_string(p, NULL, 0);

	int passed = fun_error_is_error(r.error);

	print_test_result("fun_platform_to_string: NULL buffer", passed);
}

// ============================================================================
// Main
// ============================================================================

int main(void)
{
	printf("=== Platform Tests ===\n\n");

	printf("-- fun_platform_get tests --\n");
	test_fun_platform_get_fills_struct();

	printf("\n-- fun_platform_os_to_string tests --\n");
	test_fun_platform_os_to_string_windows();
	test_fun_platform_os_to_string_linux();
	test_fun_platform_os_to_string_darwin();
	test_fun_platform_os_to_string_unknown();
	test_fun_platform_os_to_string_null();

	printf("\n-- fun_platform_arch_to_string tests --\n");
	test_fun_platform_arch_to_string_amd64();
	test_fun_platform_arch_to_string_arm64();
	test_fun_platform_arch_to_string_unknown();
	test_fun_platform_arch_to_string_null();

	printf("\n-- fun_platform_to_string tests --\n");
	test_fun_platform_to_string_format();
	test_fun_platform_to_string_null();

	printf("\n=== Tests Complete ===\n");

	return 0;
}
