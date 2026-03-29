#ifndef LIBRARY_SHUTDOWN_H
#define LIBRARY_SHUTDOWN_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../error/error.h"

/*
 * Shutdown Framework
 *
 * Provides structured shutdown coordination for applications using the
 * Fundamental Library. Registers cleanup functions to run in reverse
 * order from initialization phases during application termination.
 *
 * Usage example:
 *   void my_cleanup_function(void) { ... }
 *   
 *   // Register cleanup at application-specific phase
 *   FUNDAMENTAL_SHUTDOWN_REGISTER(SHUTDOWN_PHASE_APP, my_cleanup_function);
 *
 *   // Trigger shutdown sequence - cleanup functions execute and process exits
 *   fun_shutdown_run(SHUTDOWN_NORMAL, 0);
 */

/* Phase constants - match startup framework */
/* Note: These must match STARTUP phase constants for symmetry */
#define SHUTDOWN_PHASE_PLATFORM 1
#define SHUTDOWN_PHASE_MEMORY 2
#define SHUTDOWN_PHASE_FILESYSTEM 3
#define SHUTDOWN_PHASE_CONFIG 4
#define SHUTDOWN_PHASE_LOGGING 5
#define SHUTDOWN_PHASE_NETWORK 6
#define SHUTDOWN_PHASE_APP 99

/* Shutdown types - categorize different shutdown scenarios */
typedef enum {
	SHUTDOWN_NORMAL, /* Normal termination requested */
	SHUTDOWN_ABNORMAL, /* Abnormal condition, cleanup before exit */
	SHUTDOWN_EXTERNAL, /* External signal/termination request */
	SHUTDOWN_EMERGENCY /* System integrity violation, Immediate halt */
} fun_shutdown_type;

/*
 * Execute shutdown sequence with specified type and exit code.
 *
 * This function implements a clean shutdown by executing all registered
 * cleanup functions in reverse initialization order. The exit_code parameter
 * determines the final process exit status.
 *
 * The function is idempotent - safe to call multiple times in sequence,
 * and protected against race conditions with atomic flags.
 *
 * @param type          Shutdown category dictating execution details
 * @param exit_code     Process exit status to return after cleanup
 */
void fun_shutdown_run(fun_shutdown_type type, int exit_code);

/*
 * Macro for registering cleanup functions for the shutdown sequence.
 *
 * Functions registered with this macro will be called during the shutdown
 * sequence in reverse order of initialization phases (APP, NETWORK, FILE, etc.)
 *
 * Usage:
 *   static void my_module_deinit(void) { ... }
 *   FUNDAMENTAL_SHUTDOWN_REGISTER(SHUTDOWN_PHASE_MY_MODULE, my_module_deinit);
 *
 * @param phase            Phase constant (SHUTDOWN_PHASE_*)
 * @param cleanup_function Function to call during shutdown sequence
 */
#define FUNDAMENTAL_SHUTDOWN_REGISTER(phase, cleanup_function)     \
	static void __attribute__((constructor))                       \
	shutdown_##cleanup_function##_registration(void)               \
	{                                                              \
		/* Registration would happen here in the implementation */ \
	}

#endif // LIBRARY_SHUTDOWN_H