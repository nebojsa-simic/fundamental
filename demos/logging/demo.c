// Minimal Logging Demo - Fundamental Library
// See build-windows-amd64.bat for compile command

#define FUNDAMENTAL_LOG_LEVEL LOG_LEVEL_DEBUG
#define FUNDAMENTAL_LOG_OUTPUT_CONSOLE 1
#define FUNDAMENTAL_LOG_OUTPUT_FILE 0

#include "fundamental/logging/logging.h"

int main(void)
{
	log_info("Application started", NULL, 0);

	StringTemplateParam debug_params[] = { { "value", { .intValue = 42 } } };
	log_debug("Debug: value = #{value}", debug_params, 1);

	StringTemplateParam warn_params[] = { { "name",
											{ .stringValue = "test" } } };
	log_warn("Warning: ${name}", warn_params, 1);

	StringTemplateParam error_params[] = { { "code", { .intValue = 500 } } };
	log_error("Error code: #{code}", error_params, 1);

	log_info("Application finished", NULL, 0);
	return 0;
}
