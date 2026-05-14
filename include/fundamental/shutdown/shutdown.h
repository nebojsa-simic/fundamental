#pragma once
#include <stdint.h>

typedef enum {
	SHUTDOWN_NORMAL = 0,
	SHUTDOWN_ABNORMAL = 1,
	SHUTDOWN_EXTERNAL = 2,
	SHUTDOWN_EMERGENCY = 3,
} fun_shutdown_type;

#define SHUTDOWN_PHASE_PLATFORM 1
#define SHUTDOWN_PHASE_MEMORY 2
#define SHUTDOWN_PHASE_FILESYSTEM 3
#define SHUTDOWN_PHASE_CONFIG 4
#define SHUTDOWN_PHASE_NETWORK 5
#define SHUTDOWN_PHASE_APP 99

void fun_shutdown_run(fun_shutdown_type type, int exit_code);
void shutdown_register_cleanup(int phase, void (*handler)(void));

#define FUNDAMENTAL_SHUTDOWN_REGISTER(phase, cleanup_function) \
	__attribute__((constructor(101))) static void              \
		shutdown_##cleanup_function##_reg(void) {              \
		shutdown_register_cleanup(phase, cleanup_function);    \
	}
