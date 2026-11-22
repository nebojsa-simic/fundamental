/*
 * INI config end-to-end demo.
 *
 * Loads ini-config.ini from the executable directory through
 * fun_config_load() and verifies that every key=value line is retrievable.
 *
 * Regression demo for: "INI parser (fun_ini_parse) only stores the first
 * key=value line".
 */

#include "fundamental/config/config.h"
#include "fundamental/console/console.h"
#include "fundamental/error/error.h"
#include "fundamental/string/string.h"

typedef struct {
	const char *key;
	const char *expected;
} ExpectedKey;

static int check_key(Config *config, const char *key, const char *expected)
{
	StringResult value = fun_config_get_string(config, key);

	fun_console_write("  ");
	fun_console_write(key);
	fun_console_write(" = ");

	if (fun_error_is_error(value.error)) {
		fun_console_write_line("<MISSING>");
		return 0;
	}

	fun_console_write_line(value.value);

	if (fun_string_compare(value.value, expected) != 0) {
		fun_console_write("    expected: ");
		fun_console_write_line(expected);
		return 0;
	}

	return 1;
}

int cli_main(int argc, const char **argv)
{
	(void)argc;
	(void)argv;

	const ExpectedKey expected[] = {
		{ "name", "fun" },
		{ "version", "0.1.0" },
		{ "description", "CLI tool for building fundamental library projects" },
		{ "fundamental", "local" },
		{ "standard", "-nostdlib" },
		{ "flags", "-fno-builtin -fno-exceptions" },
		{ "cpu", "avx512" },
	};
	const size_t expected_count = sizeof(expected) / sizeof(expected[0]);

	fun_console_write_line("=== INI config end-to-end demo ===");
	fun_console_write_line(
		"Loading ini-config.ini via fun_config_load(\"ini-config\")");
	fun_console_write_line("");

	ConfigResult config_result = fun_config_load("ini-config", 0, NULL);
	if (fun_error_is_error(config_result.error)) {
		fun_console_write_line("FAIL: could not load config");
		return 1;
	}
	Config *config = &config_result.value;

	size_t ok_count = 0;
	for (size_t i = 0; i < expected_count; i++) {
		if (check_key(config, expected[i].key, expected[i].expected))
			ok_count++;
	}

	boolResult has_cpu = fun_config_has(config, "cpu");

	fun_console_write_line("");
	fun_console_write("fun_config_has(\"cpu\") = ");
	fun_console_write_line(has_cpu.value ? "true" : "false");

	int ok = (ok_count == expected_count && has_cpu.value);
	fun_console_write_line(ok ? "PASS: all INI keys retrieved" :
								"FAIL: INI keys missing");

	fun_config_destroy(config);
	return ok ? 0 : 1;
}
