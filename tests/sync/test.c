#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "fundamental/sync/sync.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define RED_CROSS "\033[0;31m\u2717\033[0m"

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

/* ---- Mutex create and destroy ---- */

void test_mutex_create_destroy()
{
	Mutex m = NULL;
	voidResult r = fun_mutex_create(&m);
	ASSERT_NO_ERROR(r);
	assert(m != NULL);

	r = fun_mutex_destroy(m);
	ASSERT_NO_ERROR(r);

	print_test_result(__func__);
}

void test_mutex_create_null_output()
{
	voidResult r = fun_mutex_create(NULL);
	ASSERT_ERROR(r);
	print_test_result(__func__);
}

void test_mutex_destroy_null()
{
	voidResult r = fun_mutex_destroy(NULL);
	ASSERT_NO_ERROR(r);
	print_test_result(__func__);
}

void test_mutex_lock_unlock()
{
	Mutex m = NULL;
	voidResult r = fun_mutex_create(&m);
	ASSERT_NO_ERROR(r);

	r = fun_mutex_lock(m);
	ASSERT_NO_ERROR(r);

	r = fun_mutex_unlock(m);
	ASSERT_NO_ERROR(r);

	r = fun_mutex_destroy(m);
	ASSERT_NO_ERROR(r);

	print_test_result(__func__);
}

/* ---- Mutex blocking test (requires thread) ---- */

typedef struct {
	Mutex mutex;
	volatile int locked;
	volatile int done;
} MutexBlockData;

#ifdef _WIN32
static DWORD WINAPI mutex_block_thread(LPVOID param)
#else
static void *mutex_block_thread(void *param)
#endif
{
	MutexBlockData *d = (MutexBlockData *)param;
	fun_mutex_lock(d->mutex);
	d->locked = 1;
	fun_mutex_unlock(d->mutex);
	d->done = 1;
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

void test_mutex_lock_blocks_another_thread()
{
	Mutex m = NULL;
	voidResult r = fun_mutex_create(&m);
	ASSERT_NO_ERROR(r);

	MutexBlockData data = { m, 0, 0 };

	fun_mutex_lock(m);

#ifdef _WIN32
	HANDLE thread = CreateThread(NULL, 0, mutex_block_thread, &data, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, mutex_block_thread, &data);
#endif

	/* Give thread time to block on the mutex */
#ifdef _WIN32
	Sleep(100);
#else
	{
		struct timespec ts = { 0, 100000000 };
		nanosleep(&ts, NULL);
	}
#endif

	/* Thread should be blocked, not yet locked */
	assert(data.locked == 0);

	fun_mutex_unlock(m);

	/* Wait for thread to finish */
#ifdef _WIN32
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
#else
	pthread_join(thread, NULL);
#endif

	assert(data.locked == 1);
	assert(data.done == 1);

	fun_mutex_destroy(m);
	print_test_result(__func__);
}

/* ---- CondVar create and destroy ---- */

void test_condvar_create_destroy()
{
	CondVar cv = NULL;
	voidResult r = fun_condvar_create(&cv);
	ASSERT_NO_ERROR(r);
	assert(cv != NULL);

	r = fun_condvar_destroy(cv);
	ASSERT_NO_ERROR(r);

	print_test_result(__func__);
}

void test_condvar_create_null_output()
{
	voidResult r = fun_condvar_create(NULL);
	ASSERT_ERROR(r);
	print_test_result(__func__);
}

/* ---- CondVar wait/signal (requires thread) ---- */

typedef struct {
	Mutex mutex;
	CondVar cv;
	volatile int ready;
	volatile int woken;
} CondVarSignalData;

#ifdef _WIN32
static DWORD WINAPI condvar_wait_thread(LPVOID param)
#else
static void *condvar_wait_thread(void *param)
#endif
{
	CondVarSignalData *d = (CondVarSignalData *)param;
	fun_mutex_lock(d->mutex);
	d->ready = 1;
	fun_condvar_wait(d->cv, d->mutex);
	d->woken = 1;
	fun_mutex_unlock(d->mutex);
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

void test_condvar_wait_signal()
{
	Mutex m = NULL;
	CondVar cv = NULL;
	voidResult r = fun_mutex_create(&m);
	ASSERT_NO_ERROR(r);
	r = fun_condvar_create(&cv);
	ASSERT_NO_ERROR(r);

	CondVarSignalData data = { m, cv, 0, 0 };

#ifdef _WIN32
	HANDLE thread = CreateThread(NULL, 0, condvar_wait_thread, &data, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, condvar_wait_thread, &data);
#endif

	/* Wait for thread to be ready and blocked on condvar */
#ifdef _WIN32
	Sleep(100);
#else
	{
		struct timespec ts = { 0, 100000000 };
		nanosleep(&ts, NULL);
	}
#endif

	assert(data.ready == 1);
	assert(data.woken == 0);

	fun_mutex_lock(m);
	fun_condvar_signal(cv);
	fun_mutex_unlock(m);

	/* Wait for thread to wake */
#ifdef _WIN32
	Sleep(100);
#else
	{
		struct timespec ts = { 0, 100000000 };
		nanosleep(&ts, NULL);
	}
#endif

	assert(data.woken == 1);

#ifdef _WIN32
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
#else
	pthread_join(thread, NULL);
#endif

	fun_condvar_destroy(cv);
	fun_mutex_destroy(m);
	print_test_result(__func__);
}

/* ---- CondVar broadcast (requires multiple threads) ---- */

typedef struct {
	Mutex mutex;
	CondVar cv;
	volatile int ready_count;
	volatile int woken_count;
	int total_threads;
} CondVarBroadcastData;

#ifdef _WIN32
static DWORD WINAPI condvar_broadcast_thread(LPVOID param)
#else
static void *condvar_broadcast_thread(void *param)
#endif
{
	CondVarBroadcastData *d = (CondVarBroadcastData *)param;
	fun_mutex_lock(d->mutex);
	d->ready_count++;
	fun_condvar_wait(d->cv, d->mutex);
	d->woken_count++;
	fun_mutex_unlock(d->mutex);
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

void test_condvar_broadcast()
{
	Mutex m = NULL;
	CondVar cv = NULL;
	voidResult r = fun_mutex_create(&m);
	ASSERT_NO_ERROR(r);
	r = fun_condvar_create(&cv);
	ASSERT_NO_ERROR(r);

	CondVarBroadcastData data = { m, cv, 0, 0, 3 };

#ifdef _WIN32
	HANDLE threads[3];
	for (int i = 0; i < 3; i++) {
		threads[i] =
			CreateThread(NULL, 0, condvar_broadcast_thread, &data, 0, NULL);
	}
#else
	pthread_t threads[3];
	for (int i = 0; i < 3; i++) {
		pthread_create(&threads[i], NULL, condvar_broadcast_thread, &data);
	}
#endif

	/* Wait for all threads to be blocked */
#ifdef _WIN32
	Sleep(200);
#else
	{
		struct timespec ts = { 0, 200000000 };
		nanosleep(&ts, NULL);
	}
#endif

	assert(data.ready_count == 3);
	assert(data.woken_count == 0);

	fun_mutex_lock(m);
	fun_condvar_broadcast(cv);
	fun_mutex_unlock(m);

	/* Wait for all threads to wake */
#ifdef _WIN32
	Sleep(100);
#else
	{
		struct timespec ts = { 0, 100000000 };
		nanosleep(&ts, NULL);
	}
#endif

	assert(data.woken_count == 3);

#ifdef _WIN32
	for (int i = 0; i < 3; i++) {
		WaitForSingleObject(threads[i], INFINITE);
		CloseHandle(threads[i]);
	}
#else
	for (int i = 0; i < 3; i++) {
		pthread_join(threads[i], NULL);
	}
#endif

	fun_condvar_destroy(cv);
	fun_mutex_destroy(m);
	print_test_result(__func__);
}

int main(void)
{
	printf("\n--- Sync Module Tests ---\n");

	test_mutex_create_destroy();
	test_mutex_create_null_output();
	test_mutex_destroy_null();
	test_mutex_lock_unlock();
	test_mutex_lock_blocks_another_thread();
	test_condvar_create_destroy();
	test_condvar_create_null_output();
	test_condvar_wait_signal();
	test_condvar_broadcast();

	printf("\nAll sync tests passed.\n");
	return 0;
}
