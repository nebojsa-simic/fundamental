/*
 * Linux Signal Handler
 * Implements SIGTERM, SIGINT handling for graceful shutdown
 */

#include "fundamental/shutdown/shutdown.h"
#include <signal.h>
#include <stdlib.h>

/*
 * POSIX Signal Handler
 * Called on SIGTERM, SIGINT, etc.
 */
static void posix_signal_handler(int signum)
{
	switch (signum) {
	case SIGTERM:
		/* External termination request */
		fun_shutdown_run(SHUTDOWN_EXTERNAL, 0);
		break;

	case SIGINT:
		/* Interrupt from keyboard (Ctrl+C) */
		fun_shutdown_run(SHUTDOWN_EXTERNAL, 130); /* 128 + 2 */
		break;

	default:
		/* Unknown signal - abnormal shutdown */
		fun_shutdown_run(SHUTDOWN_ABNORMAL, 1);
		break;
	}
}

/*
 * Install POSIX signal handlers
 * Called during startup to register signal handlers
 */
void fun_signals_install_handlers(void)
{
	/* Setup SIGTERM handler */
	signal(SIGTERM, posix_signal_handler);

	/* Setup SIGINT handler */
	signal(SIGINT, posix_signal_handler);
}

/*
 * Platform-specific exit function for Linux
 * Called by shutdown framework after cleanup
 */
void platform_shutdown_exit(int exit_code)
{
	exit(exit_code);
}
