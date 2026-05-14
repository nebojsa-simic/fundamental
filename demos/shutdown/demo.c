/*
 * Shutdown Framework Demo
 */

#include "fundamental/shutdown/shutdown.h"
#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/string/string.h"
#include "fundamental/async/async.h"
#include "fundamental/startup/startup.h"

int fun_network_init(void) { return 0; }

static const char *CLEANUP_FILE = "shutdown_cleanup.txt";

static void app_cleanup(void)
{
	const char *content = "Shutdown framework demo: cleanup function executed!\n";
	Write params = { .file_path = CLEANUP_FILE,
					 .input = (Memory)content,
					 .bytes_to_write = fun_string_length(content),
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_SYNC };
	AsyncResult write_result = fun_write_memory_to_file(params);
	fun_async_await(&write_result, -1);
	if (fun_error_is_ok(write_result.error)) {
		fun_console_write_line("");
		fun_console_write_line("=== Cleanup Executed ===");
		fun_console_write_line("File written: shutdown_cleanup.txt");
	}
}

FUNDAMENTAL_SHUTDOWN_REGISTER(SHUTDOWN_PHASE_APP, app_cleanup);

int main(void)
{
	fun_startup_run();

	fun_console_write_line("=== Shutdown Framework Demo ===");
	fun_console_write_line("Press Ctrl+C during countdown...");
	fun_console_write_line("");

	for (int i = 10; i > 0; i--) {
		char msg[128];
		StringTemplateParam params[] = { { "seconds", { .intValue = i } } };
		fun_string_template("#{seconds} seconds remaining...", params, 1, msg, sizeof(msg));
		fun_console_write_line(msg);
		for (volatile int j = 0; j < 100000000; j++);
	}

	fun_console_write_line("Timeout - normal exit");
	return 0;
}
