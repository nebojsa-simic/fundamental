#include "fundamental/shutdown/shutdown.h"
#include "fundamental/arch/atomic.h"

extern void platform_shutdown_exit(int exit_code);

typedef struct {
	int phase;
	void (*func)(void);
	int valid;
} fun_shutdown_entry;

#define MAX_SHUTDOWN_ENTRIES 32
static fun_shutdown_entry shutdown_entries[MAX_SHUTDOWN_ENTRIES];
static int shutdown_entry_count = 0;
static int shutdown_started = 0;

static void execute_shutdown_phases(fun_shutdown_type type, int exit_code);
static void execute_shutdown_phase(int phase);

void fun_shutdown_run(fun_shutdown_type type, int exit_code)
{
	int expected = 0;
	if (!fun_atomic_compare_and_swap(&shutdown_started, &expected, 1)) {
		return;
	}
	execute_shutdown_phases(type, exit_code);
	platform_shutdown_exit(exit_code);
}

static void execute_shutdown_phases(fun_shutdown_type type, int exit_code)
{
	for (int phase = 99; phase >= 1; phase--) {
		execute_shutdown_phase(phase);
	}
}

static void execute_shutdown_phase(int phase)
{
	for (int i = 0; i < shutdown_entry_count; i++) {
		if (shutdown_entries[i].valid && shutdown_entries[i].phase == phase) {
			shutdown_entries[i].func();
		}
	}
}

void shutdown_register_cleanup(int phase, void (*handler)(void))
{
	int pos = shutdown_entry_count++;
	if (pos < MAX_SHUTDOWN_ENTRIES) {
		shutdown_entries[pos].phase = phase;
		shutdown_entries[pos].func = handler;
		shutdown_entries[pos].valid = 1;
	}
}
