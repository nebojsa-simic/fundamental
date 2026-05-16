#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "fundamental/thread_pool/thread_pool.h"
#include "fundamental/error/error.h"

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

/* ---- Helper: platform sleep ---- */

static void sleep_ms(int ms)
{
#ifdef _WIN32
	Sleep(ms);
#else
	{
		struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
		nanosleep(&ts, NULL);
	}
#endif
}

/* ---- Shared globals for work function coordination ---- */

static volatile int g_work_value;
static volatile int g_work_executed;
static volatile int g_block_workers;
static void *g_worker_received_ptr;

static void set_executed_work_fn(void *data)
{
	(void)data;
	g_work_executed = 1;
}

static void check_copy_work_fn(void *data)
{
	g_worker_received_ptr = data;
	if (data != NULL) {
		g_work_value = *(int *)data;
	}
	g_work_executed = 1;
}

static void spin_work_fn(void *data)
{
	(void)data;
	while (g_block_workers) {
	}
}

/* ================================================================
   8.1  Pool creation with valid parameters
   ================================================================ */
void test_create_valid()
{
	g_work_executed = 0;
	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(4, &pool);
	ASSERT_NO_ERROR(r);
	assert(pool != NULL);

	fun_thread_pool_destroy(pool);
	print_test_result(__func__);
}

/* ================================================================
   8.2  Pool creation with num_threads=0 returns error
   ================================================================ */
void test_create_zero_threads()
{
	ThreadPool pool = (ThreadPool)0xDEAD;
	voidResult r = fun_thread_pool_create(0, &pool);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_THREAD_POOL_INVALID_SIZE);
	assert(pool == (ThreadPool)0xDEAD);
	print_test_result(__func__);
}

/* ================================================================
   8.3  Pool creation with NULL out_pool returns error
   ================================================================ */
void test_create_null_output()
{
	voidResult r = fun_thread_pool_create(2, NULL);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_NULL_POINTER);
	print_test_result(__func__);
}

/* ================================================================
   8.4  Submit succeeds, worker receives copied data
   ================================================================ */
void test_submit_data_copied()
{
	g_worker_received_ptr = NULL;
	g_work_executed = 0;
	g_work_value = 0;

	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(1, &pool);
	ASSERT_NO_ERROR(r);

	int original = 42;
	WorkItem item = { &original, sizeof(original), check_copy_work_fn };

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);

	fun_thread_pool_destroy(pool);

	assert(g_work_executed == 1);
	assert(g_worker_received_ptr != NULL);
	assert(g_worker_received_ptr != &original);
	assert(g_work_value == 42);

	print_test_result(__func__);
}

/* ================================================================
   8.5  Caller frees original data after submit, worker unaffected
   ================================================================ */
void test_caller_frees_after_submit()
{
	g_work_executed = 0;

	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(1, &pool);
	ASSERT_NO_ERROR(r);

	int original = 99;
	WorkItem item = { &original, sizeof(original), set_executed_work_fn };

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);

	original = 0;

	fun_thread_pool_destroy(pool);

	assert(g_work_executed == 1);

	print_test_result(__func__);
}

/* ================================================================
   8.6  Submit returns THREAD_POOL_FULL when all workers busy
   ================================================================ */
void test_submit_pool_full()
{
	g_block_workers = 1;

	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(2, &pool);
	ASSERT_NO_ERROR(r);

	int dummy = 0;
	WorkItem item = { &dummy, sizeof(dummy), spin_work_fn };

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);
	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);

	sleep_ms(50);

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);
	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);

	sleep_ms(50);

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_THREAD_POOL_FULL);

	g_block_workers = 0;

	fun_thread_pool_destroy(pool);
	print_test_result(__func__);
}

/* ================================================================
    8.7  THREAD_POOL_FULL - no internal copy, caller retains ownership
   ================================================================ */
void test_submit_full_retains_ownership()
{
	g_block_workers = 1;

	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(1, &pool);
	ASSERT_NO_ERROR(r);

	int original = 55;
	WorkItem item = { &original, sizeof(original), spin_work_fn };

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);

	sleep_ms(50);

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);

	sleep_ms(50);

	original = 77;

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_THREAD_POOL_FULL);

	assert(original == 77);

	g_block_workers = 0;

	fun_thread_pool_destroy(pool);
	print_test_result(__func__);
}

/* ================================================================
    8.8  Submit with NULL pool returns error
   ================================================================ */
void test_submit_null_pool()
{
	int data = 1;
	WorkItem item = { &data, sizeof(data), set_executed_work_fn };
	voidResult r = fun_thread_pool_submit(NULL, &item);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_NULL_POINTER);
	print_test_result(__func__);
}

/* ================================================================
   8.9  Submit with NULL item returns error
   ================================================================ */
void test_submit_null_item()
{
	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(1, &pool);
	ASSERT_NO_ERROR(r);

	r = fun_thread_pool_submit(pool, NULL);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_NULL_POINTER);

	fun_thread_pool_destroy(pool);
	print_test_result(__func__);
}

/* ================================================================
   8.10  Submit with NULL item->data returns error
   ================================================================ */
void test_submit_null_data()
{
	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(1, &pool);
	ASSERT_NO_ERROR(r);

	WorkItem item = { NULL, 4, set_executed_work_fn };
	r = fun_thread_pool_submit(pool, &item);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_NULL_POINTER);

	fun_thread_pool_destroy(pool);
	print_test_result(__func__);
}

/* ================================================================
   8.11  Submit with NULL item->work_fn returns error
   ================================================================ */
void test_submit_null_work_fn()
{
	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(1, &pool);
	ASSERT_NO_ERROR(r);

	int data = 1;
	WorkItem item = { &data, sizeof(data), NULL };
	r = fun_thread_pool_submit(pool, &item);
	ASSERT_ERROR(r);
	assert(r.error.code == ERROR_CODE_NULL_POINTER);

	fun_thread_pool_destroy(pool);
	print_test_result(__func__);
}

/* ================================================================
   8.12  Destroy on NULL pool is no-op
   ================================================================ */
void test_destroy_null_pool()
{
	voidResult r = fun_thread_pool_destroy(NULL);
	ASSERT_NO_ERROR(r);
	print_test_result(__func__);
}

/* ================================================================
   8.13  Destroy waits for in-progress work to complete
   ================================================================ */
void test_destroy_waits_for_work()
{
	g_work_executed = 0;

	ThreadPool pool = NULL;
	voidResult r = fun_thread_pool_create(1, &pool);
	ASSERT_NO_ERROR(r);

	int data = 1;
	WorkItem item = { &data, sizeof(data), set_executed_work_fn };

	r = fun_thread_pool_submit(pool, &item);
	ASSERT_NO_ERROR(r);

	fun_thread_pool_destroy(pool);

	assert(g_work_executed == 1);

	print_test_result(__func__);
}

/* ================================================================
   8.14  Concurrent submits from multiple threads
   ================================================================ */
#define CONCURRENT_COUNT 100

static ThreadPool g_concurrent_pool;
static volatile int g_concurrent_executed;
static volatile int g_concurrent_errors;

static void concurrent_work_fn(void *data)
{
	(void)data;
#ifdef _WIN32
	InterlockedIncrement((LONG volatile *)&g_concurrent_executed);
#else
	__atomic_add_fetch(&g_concurrent_executed, 1, __ATOMIC_RELAXED);
#endif
}

#ifdef _WIN32
static DWORD WINAPI concurrent_submit_thread(LPVOID param)
#else
static void *concurrent_submit_thread(void *param)
#endif
{
	(void)param;

	for (int i = 0; i < CONCURRENT_COUNT; i++) {
		int data = i;
		WorkItem item = { &data, sizeof(data), concurrent_work_fn };
		voidResult r = fun_thread_pool_submit(g_concurrent_pool, &item);
		if (fun_error_is_error(r.error)) {
#ifdef _WIN32
			InterlockedIncrement((LONG volatile *)&g_concurrent_errors);
#else
			__atomic_add_fetch(&g_concurrent_errors, 1, __ATOMIC_RELAXED);
#endif
		}
	}

#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

void test_concurrent_submits()
{
	g_concurrent_executed = 0;
	g_concurrent_errors = 0;

	voidResult r = fun_thread_pool_create(4, &g_concurrent_pool);
	ASSERT_NO_ERROR(r);

#define NUM_SUBMIT_THREADS 4

#ifdef _WIN32
	HANDLE threads[NUM_SUBMIT_THREADS];
	for (int i = 0; i < NUM_SUBMIT_THREADS; i++) {
		threads[i] =
			CreateThread(NULL, 0, concurrent_submit_thread, NULL, 0, NULL);
		assert(threads[i] != NULL);
	}
#else
	pthread_t threads[NUM_SUBMIT_THREADS];
	for (int i = 0; i < NUM_SUBMIT_THREADS; i++) {
		int ret =
			pthread_create(&threads[i], NULL, concurrent_submit_thread, NULL);
		assert(ret == 0);
	}
#endif

	for (int i = 0; i < NUM_SUBMIT_THREADS; i++) {
#ifdef _WIN32
		WaitForSingleObject(threads[i], INFINITE);
		CloseHandle(threads[i]);
#else
		pthread_join(threads[i], NULL);
#endif
	}

	fun_thread_pool_destroy(g_concurrent_pool);

	int total_submitted =
		(NUM_SUBMIT_THREADS * CONCURRENT_COUNT) - g_concurrent_errors;
	assert(g_concurrent_executed == total_submitted);

	print_test_result(__func__);
}

#undef CONCURRENT_COUNT
#undef NUM_SUBMIT_THREADS

int main(void)
{
	printf("\n--- Thread Pool Tests ---\n\n");

	test_create_valid();
	test_create_zero_threads();
	test_create_null_output();
	test_submit_data_copied();
	test_caller_frees_after_submit();
	test_submit_pool_full();
	test_submit_full_retains_ownership();
	test_submit_null_pool();
	test_submit_null_item();
	test_submit_null_data();
	test_submit_null_work_fn();
	test_destroy_null_pool();
	test_destroy_waits_for_work();
	test_concurrent_submits();

	printf("\nAll thread-pool tests passed.\n");
	return 0;
}
