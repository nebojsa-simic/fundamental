/* Windows-specific process exit */

/* Windows exit function */
#include <stdint.h>

/* Declare the Windows ExitProcess function */
void ExitProcess(uint32_t exit_code); /* Implemented by Windows API */

/* Platform-specific exit function */
void platform_shutdown_exit(int exit_code)
{
	ExitProcess((uint32_t)exit_code);
}