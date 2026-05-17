/*
 * Reproducer for the residual Linux thread-pool race (~18% failure rate).
 *
 * The race: fun_thread_pool_submit() returns THREAD_POOL_FULL when a
 * slot should be idle, because a clone'd worker thread has not yet taken
 * ownership of previously submitted work (slot->data still non-NULL).
 *
 * This is NOT a scheduling problem (futex readiness handshake disproved
 * that — see tasks.md §10.1).  The root cause is an unresolved edge
 * case in the condvar/mutex futex interaction on Linux.
 *
 * Build (from tests/thread-pool/):
 *   gcc -Os --std=c17 -I ../../include reproduce_race.c \
 *       ../../arch/memory/linux-amd64/memory.c \
 *       ../../arch/sync/linux-amd64/sync.c \
 *       ../../src/thread_pool/thread_pool.c \
 *       ../../arch/thread_pool/linux-amd64/thread_pool.c \
 *       -o reproduce_race
 *
 * Expected: catches THREAD_POOL_FULL on submit 3 or 4 within ~50 trials.
 * A clean run (no failures in 500 trials) means the race is fixed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fundamental/thread_pool/thread_pool.h"
#include "fundamental/error/error.h"

/* ---- helpers ---- */

static void sleep_ms(int ms)
{
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
	nanosleep(&ts, NULL);
}

/* ---- shared with workers ---- */

static volatile int g_block = 1;

static void spin_fn(void *data)
{
	(void)data;
	while (g_block) {
	}
}

/* ---- main ---- */

int main(void)
{
	int trial;

	for (trial = 1; trial <= 500; trial++) {
		g_block = 1;

		ThreadPool pool = NULL;
		voidResult r = fun_thread_pool_create(2, &pool);
		if (r.error.code != 0) {
			fprintf(stderr, "trial %d: create failed code=%d\n", trial,
					r.error.code);
			return 1;
		}

		int payload = 0;
		WorkItem item = { &payload, sizeof(payload), spin_fn };

		/* Submits 1 & 2: fill both slots.  Workers will take
		   ownership (slot->data = NULL) then spin in spin_fn. */
		r = fun_thread_pool_submit(pool, &item);
		if (r.error.code != 0) {
			fprintf(stderr, "trial %d: submit1 code=%d\n", trial, r.error.code);
			return 1;
		}
		r = fun_thread_pool_submit(pool, &item);
		if (r.error.code != 0) {
			fprintf(stderr, "trial %d: submit2 code=%d\n", trial, r.error.code);
			return 1;
		}

		/* Brief pause so workers can take ownership. */
		sleep_ms(50);

		/* Submits 3 & 4: both slots should be idle now
		   (workers set slot->data = NULL before spinning).
		   If a worker hasn't taken ownership yet, the slot
		   still appears busy → THREAD_POOL_FULL. */
		r = fun_thread_pool_submit(pool, &item);
		if (r.error.code != 0) {
			fprintf(stderr,
					"RACE-HIT trial %d: submit3 code=%d "
					"(expected 0)\n",
					trial, r.error.code);
			g_block = 0;
			fun_thread_pool_destroy(pool);
			return 1;
		}
		r = fun_thread_pool_submit(pool, &item);
		if (r.error.code != 0) {
			fprintf(stderr,
					"RACE-HIT trial %d: submit4 code=%d "
					"(expected 0)\n",
					trial, r.error.code);
			g_block = 0;
			fun_thread_pool_destroy(pool);
			return 1;
		}

		/* Submit 5 should be THREAD_POOL_FULL. */
		r = fun_thread_pool_submit(pool, &item);
		if (r.error.code != ERROR_CODE_THREAD_POOL_FULL) {
			fprintf(stderr,
					"RACE-HIT trial %d: submit5 code=%d "
					"(expected %d = FULL)\n",
					trial, r.error.code, ERROR_CODE_THREAD_POOL_FULL);
			g_block = 0;
			fun_thread_pool_destroy(pool);
			return 1;
		}

		g_block = 0;
		fun_thread_pool_destroy(pool);

		if (trial % 50 == 0)
			fprintf(stderr, "  ... %d trials, no hit yet\n", trial);
	}

	fprintf(stderr, "NO-RACE: %d trials without failure\n", trial - 1);
	return 0;
}
