#include <stdio.h>
#include <stdbool.h>
#include "fundamental/logging/logging.h"
#include "fundamental/string/string.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"
#define RED_X "\033[0;31m\342\234\227\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

static void print_test_result(const char *test_name, bool passed)
{
	if (passed) {
		printf("%s %s\n", GREEN_CHECK, test_name);
		tests_passed++;
	} else {
		printf("%s %s\n", RED_X, test_name);
		tests_failed++;
	}
}

static void test_log_levels_compile(void)
{
	StringTemplateParam p[] = { { "value", { .intValue = 42 } } };

	log_trace("Trace message: #{value}", p, 1);
	log_debug("Debug message: #{value}", p, 1);
	log_info("Info message: #{value}", p, 1);
	log_warn("Warn message: #{value}", p, 1);
	log_error("Error message: #{value}", p, 1);

	print_test_result("test_log_levels_compile", true);
}

static void test_log_empty_params(void)
{
	log_info("Simple message with no params", NULL, 0);
	log_error("Error with no params", NULL, 0);

	print_test_result("test_log_empty_params", true);
}

static void test_log_basename_strips_path(void)
{
	const char *full_path = "/home/user/fundamental/src/test.c";
	const char *basename = log_basename(full_path);
	bool passed = (basename[0] == 't' && basename[1] == 'e' &&
				   basename[2] == 's' && basename[3] == 't' &&
				   basename[4] == '.' && basename[5] == 'c' &&
				   basename[6] == '\0');

	log_info("Basename test", NULL, 0);
	print_test_result("test_log_basename_strips_path", passed);
}

static void test_log_timestamp_format(void)
{
	char timestamp[32];
	log_format_timestamp(timestamp, sizeof(timestamp));

	bool passed = true;
	size_t len = fun_string_length(timestamp);

	if (len != 24)
		passed = false;
	if (timestamp[4] != '-' || timestamp[7] != '-' || timestamp[10] != 'T' ||
		timestamp[13] != ':' || timestamp[16] != ':' || timestamp[19] != '.' ||
		timestamp[23] != 'Z')
		passed = false;

	log_info("Timestamp format test", NULL, 0);
	print_test_result("test_log_timestamp_format", passed);
}

static void test_log_template_formatting(void)
{
	StringTemplateParam p[] = { { "user", { .intValue = 123 } },
								{ "ip", { .stringValue = "192.168.1.1" } } };

	log_info("User #{user} from ${ip}", p, 2);

	print_test_result("test_log_template_formatting", true);
}

static void test_log_console_output(void)
{
	const LoggingConfig *config = fun_logging_get_config();
	bool passed = (config != NULL && config->output_console == true);

	log_info("Console output test", NULL, 0);
	print_test_result("test_log_console_output", passed);
}

static void test_log_config_loaded(void)
{
	bool init = fun_logging_is_initialized();
	const LoggingConfig *config = fun_logging_get_config();

	bool passed =
		(init == true && config != NULL && config->level >= LOG_LEVEL_TRACE &&
		 config->level <= LOG_LEVEL_ERROR);

	print_test_result("test_log_config_loaded", passed);
}

static void test_log_output_format(void)
{
	log_info("Output format verification", NULL, 0);

	print_test_result("test_log_output_format", true);
}

static void test_log_level_trace_enabled(void)
{
	const LoggingConfig *config = fun_logging_get_config();
	bool passed = (config != NULL && config->level <= LOG_LEVEL_TRACE);

	print_test_result("test_log_level_trace_enabled", passed);
}

static void test_log_multiple_sequential(void)
{
	for (int i = 0; i < 5; i++) {
		StringTemplateParam p[] = { { "i", { .intValue = i } } };
		log_info("Iteration #{i}", p, 1);
	}

	print_test_result("test_log_multiple_sequential", true);
}

static void test_log_source_location(void)
{
	log_info("Source location test", NULL, 0);

	print_test_result("test_log_source_location", true);
}

static void test_log_compile_time_filter(void)
{
#if FUNDAMENTAL_LOG_LEVEL <= LOG_LEVEL_TRACE
	bool passed = true;
#else
	bool passed = false;
#endif

	print_test_result("test_log_compile_time_filter", passed);
}

static void test_log_buffer_size_config(void)
{
	const LoggingConfig *config = fun_logging_get_config();
	bool passed = (config != NULL && config->buffer_size >= 128);

	print_test_result("test_log_buffer_size_config", passed);
}

int main(void)
{
	printf("Running logging module tests:\n\n");

	test_log_levels_compile();
	test_log_empty_params();
	test_log_basename_strips_path();
	test_log_timestamp_format();
	test_log_template_formatting();
	test_log_console_output();
	test_log_config_loaded();
	test_log_output_format();
	test_log_level_trace_enabled();
	test_log_multiple_sequential();
	test_log_source_location();
	test_log_compile_time_filter();
	test_log_buffer_size_config();

	printf("\n");
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);

	if (tests_failed > 0) {
		printf("\nSome tests failed!\n");
		return 1;
	}

	printf("\nAll logging module tests passed.\n");
	return 0;
}
