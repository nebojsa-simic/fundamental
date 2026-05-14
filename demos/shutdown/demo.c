/*
 * Shutdown Framework Demo
 * Demonstrates graceful shutdown with cleanup on Ctrl+C
 *
 * This demo:
 * 1. Registers a cleanup function that writes a file
 * 2. Waits for 10 seconds (simulating work)
 * 3. If Ctrl+C is pressed, shutdown handlers run and write cleanup file
 * 4. If timeout completes, normal exit without cleanup file
 */

#include "fundamental/shutdown/shutdown.h"
#include "fundamental/console/console.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/string/string.h"
#include "fundamental/async/async.h"

static const char *CLEANUP_FILE = "shutdown_cleanup.txt";
static int cleanup_ran = 0;

/*
 * Cleanup function registered with shutdown framework
 * This runs when Ctrl+C is pressed OR during normal shutdown
 */
static void app_cleanup(void)
{
	cleanup_ran = 1;

	/* Write a file to demonstrate cleanup ran */
	const char *content =
		"Shutdown framework demo: cleanup function executed!\n";

	Write params = { .file_path = CLEANUP_FILE,
					 .input = (Memory)content,
					 .bytes_to_write = fun_string_length(content),
					 .offset = 0,
					 .mode = FILE_MODE_AUTO,
					 .durability_mode = FILE_DURABILITY_SYNC };

	AsyncResult write_result = fun_write_memory_to_file(params);
	fun_async_await(&write_result, -1);

	if (fun_error_is_ok(write_result.error)) {
		fun_console_write_line("");
		fun_console_write_line("=== Cleanup Executed ===");
		fun_console_write_line("File written: ");
		fun_console_write_line(CLEANUP_FILE);
		fun_console_write_line("This proves shutdown handlers ran!");
	} else {
		fun_console_write_line("Cleanup: Failed to write file");
	}
}

/* Register cleanup function at APP phase */
FUNDAMENTAL_SHUTDOWN_REGISTER(SHUTDOWN_PHASE_APP, app_cleanup);

int main(void)
{
	fun_console_write_line("=== Shutdown Framework Demo ===");
	fun_console_write_line("");
	fun_console_write_line("This demo waits 10 seconds.");
	fun_console_write_line("Press Ctrl+C to trigger graceful shutdown.");
	fun_console_write_line("");
	fun_console_write_line("Starting countdown...");
	fun_console_write_line("");

	/* Simulate work with countdown */
	for (int i = 10; i > 0; i--) {
		char msg[128];
		StringTemplateParam params[] = { { "seconds", { .intValue = i } } };

		fun_string_template("#{seconds} seconds remaining...", params, 1, msg,
							sizeof(msg));
		fun_console_write_line(msg);

		/* Wait 1 second using async sleep (simplified: just loop) */
		/* In real app, use proper async wait */
		for (volatile int j = 0; j < 100000000; j++)
			; /* Busy wait ~1 second */
	}

	fun_console_write_line("");
	fun_console_write_line("Timeout completed - normal exit");
	fun_console_write_line(
		"(Cleanup did NOT run - only runs on shutdown signal)");

	return 0;
}
