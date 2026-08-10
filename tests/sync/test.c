#define _POSIX_C_SOURCE 199309L
#include <time.h>

#include "fundamental/sync/sync.h"
#include "fundamental/console/console.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"
#define RED_CROSS "\033[0;31m\u2717\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

/* ---- Mutex create and destroy ---- */

void test_mutex_create_destroy()
{
	Mutex m = NULL;
	voidResult r = fun_mutex_create(&m);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(m != NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	r = fun_mutex_destroy(m);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result(__func__);
}

void test_mutex_create_null_output()
{
	voidResult r = fun_mutex_create(NULL);
	if (r.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
	print_test_result(__func__);
}

void test_mutex_destroy_null()
{
	voidResult r = fun_mutex_destroy(NULL);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	print_test_result(__func__);
}

void test_mutex_lock_unlock()
{
	Mutex m = NULL;
	voidResult r = fun_mutex_create(&m);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	r = fun_mutex_lock(m);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	r = fun_mutex_unlock(m);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	r = fun_mutex_destroy(m);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

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
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

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
	if (!(data.locked == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	fun_mutex_unlock(m);

	/* Wait for thread to finish */
#ifdef _WIN32
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
#else
	pthread_join(thread, NULL);
#endif

	if (!(data.locked == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(data.done == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	fun_mutex_destroy(m);
	print_test_result(__func__);
}

/* ---- CondVar create and destroy ---- */

void test_condvar_create_destroy()
{
	CondVar cv = NULL;
	voidResult r = fun_condvar_create(&cv);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	if (!(cv != NULL)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

	r = fun_condvar_destroy(cv);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

	print_test_result(__func__);
}

void test_condvar_create_null_output()
{
	voidResult r = fun_condvar_create(NULL);
	if (r.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
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
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	r = fun_condvar_create(&cv);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

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

	if (!(data.ready == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(data.woken == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

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

	if (!(data.woken == 1)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

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
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}
	r = fun_condvar_create(&cv);
	if (r.error.code != 0) {
		fun_console_write_line("FAIL: ASSERT_NO_ERROR");
		return;
	}

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

	if (!(data.ready_count == 3)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(data.woken_count == 0)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

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

	if (!(data.woken_count == 3)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}

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
	fun_console_write_line("");
	fun_console_write_line("--- Sync Module Tests ---");

	test_mutex_create_destroy();
	test_mutex_create_null_output();
	test_mutex_destroy_null();
	test_mutex_lock_unlock();
	test_mutex_lock_blocks_another_thread();
	test_condvar_create_destroy();
	test_condvar_create_null_output();
	test_condvar_wait_signal();
	test_condvar_broadcast();

	fun_console_write_line("");
	fun_console_write_line("All sync tests passed.");
	return 0;
}
