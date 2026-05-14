/*
 * Windows Signal/Console Event Handler
 * Implements Ctrl+C and Ctrl+Break handling for graceful shutdown
 */

#include "fundamental/shutdown/shutdown.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 * Windows Console Control Handler
 * Called when user presses Ctrl+C or Ctrl+Break
 */
static BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
	switch (ctrl_type) {
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
		/* Trigger external shutdown with exit code 130 (128 + SIGINT) */
		fun_shutdown_run(SHUTDOWN_EXTERNAL, 130);
		return TRUE; /* Event handled */

	case CTRL_CLOSE_EVENT:
		/* Console window is closing */
		fun_shutdown_run(SHUTDOWN_EXTERNAL, 0);
		return TRUE;

	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		/* System shutdown/logoff - emergency shutdown */
		fun_shutdown_run(SHUTDOWN_EMERGENCY, 1);
		return TRUE;

	default:
		return FALSE; /* Let other handlers process */
	}
}

/*
 * Install Windows console event handlers
 * Called during startup to register signal handlers
 */
void fun_signals_install_handlers(void)
{
	SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
}

/*
 * Platform-specific exit function for Windows
 * Called by shutdown framework after cleanup
 */
void platform_shutdown_exit(int exit_code)
{
	ExitProcess((UINT)exit_code);
}
