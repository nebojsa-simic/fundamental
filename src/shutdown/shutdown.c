#include "fundamental/shutdown/shutdown.h"
#include "fundamental/arch/atomic.h"

/* Structure to hold registered shutdown function for each phase */
typedef struct {
	int phase;
	void (*func)(void);
	int valid; /* 1 if this entry is valid, 0 otherwise */
} fun_shutdown_entry;

/* Maximum number of registered functions per phase */
#define MAX_SHUTDOWN_ENTRIES 32

/* Static storage for registered shutdown functions per phase */
static fun_shutdown_entry shutdown_entries[MAX_SHUTDOWN_ENTRIES];
static int shutdown_entry_count = 0;

/* Static variables for idempotent shutdown */
static int shutdown_started = 0;
static int shutdown_completed = 0;

/* Forward declaration for shutdown phase handling */
static void execute_shutdown_phases(fun_shutdown_type type, int exit_code);
static void execute_shutdown_phase(int phase);

void fun_shutdown_run(fun_shutdown_type type, int exit_code)
{
	int expected = 0;

	/* Atomic compare and exchange to ensure only one shutdown sequence starts */
	if (!fun_atomic_compare_and_swap(&shutdown_started, &expected, 1)) {
		/* Another thread is already handling shutdown - return gracefully */
		return;
	}

	/* Execute all registered shutdown phases in reverse order */
	execute_shutdown_phases(type, exit_code);

	/* Mark shutdown as completed */
	fun_atomic_store(&shutdown_completed, 1);

	/* Architecture-specific exit deferred to platform layer */
	platform_shutdown_exit(exit_code);
}

static void execute_shutdown_phases(fun_shutdown_type type, int exit_code)
{
	/* Execute all phases in descending numerical order, i.e. reverse of init order */
	/* Go from highest phase (APP = 99) to lowest phase (PLATFORM = 1) */
	for (int phase = 99; phase >= 1; phase--) {
		execute_shutdown_phase(phase);
	}
}

static void execute_shutdown_phase(int phase)
{
	/* Execute all registered functions for this specific phase */
	int count =
		fun_atomic_fetch_and_add(&shutdown_entry_count, 0); /* Atomic read */

	for (int i = 0; i < count; i++) {
		if (shutdown_entries[i].valid && shutdown_entries[i].phase == phase &&
			shutdown_entries[i].func != 0) {
			/* Call the registered shutdown function */
			shutdown_entries[i].func();
		}
	}
}

/* Implementation of function to register shutdown handlers */
void shutdown_register_cleanup(int phase, void (*handler)(void))
{
	int pos = fun_atomic_fetch_and_add(&shutdown_entry_count, 1);
	if (pos < MAX_SHUTDOWN_ENTRIES) {
		shutdown_entries[pos].phase = phase;
		shutdown_entries[pos].func = handler;
		shutdown_entries[pos].valid = 1;
	}
}

/* Declare platform-specific exit function (implemented in platform arch/ layers) */
extern void platform_shutdown_exit(int exit_code);