#ifndef LIBRARY_TIMING_H
#define LIBRARY_TIMING_H

#include <stdint.h>

uint64_t fun_timing_now_ns(void);

/*
 * Current wall-clock time as seconds since the Unix epoch (1970-01-01 UTC).
 * Used for certificate validity checks; monotonic timing is not suitable.
 */
int64_t fun_timing_now_unix(void);

#endif
