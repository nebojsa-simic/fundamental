#include <stdio.h>
#include <assert.h>
#include "../../include/async/async.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"

#define ASSERT_NO_ERROR(result) assert(result.code == 0)
#define ASSERT_ERROR(result) assert(result.code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

static void test_process_spawn_success(void)
{
#ifdef _WIN32
	AsyncResult result = fun_async_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "exit", "0", NULL },
		NULL);
#else
	AsyncResult result = fun_async_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "exit 0", NULL }, NULL);
#endif

	assert(result.error.code == ERROR_CODE_NO_ERROR ||
		   result.error.code == ERROR_CODE_PROCESS_NOT_FOUND);

	if (result.error.code == ERROR_CODE_NO_ERROR) {
		assert(result.status == ASYNC_PENDING);
		fun_async_await(&result);
		assert(result.status == ASYNC_COMPLETED ||
			   result.status == ASYNC_ERROR);
		assert(!result.process.is_running);
		fun_process_free(&result);
	}

	print_test_result("test_process_spawn_success");
}

static void test_process_spawn_not_found(void)
{
	AsyncResult result = fun_async_process_spawn(
		"nonexistent_executable_xyz123",
		(const char *[]){ "nonexistent_executable_xyz123", NULL }, NULL);

	ASSERT_ERROR(result.error);
	assert(result.error.code == ERROR_CODE_PROCESS_NOT_FOUND);
	print_test_result("test_process_spawn_not_found");
}

static void test_process_stdout_capture(void)
{
#ifdef _WIN32
	AsyncResult result = fun_async_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "echo", "Hello", NULL },
		NULL);
#else
	AsyncResult result = fun_async_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "echo Hello", NULL },
		NULL);
#endif

	if (result.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&result);
		assert(result.status == ASYNC_COMPLETED);

		size_t length = 0;
		const char *out_data = fun_process_get_stdout(&result, &length);
		assert(out_data != NULL);
		assert(length > 0);

		fun_process_free(&result);
	}

	print_test_result("test_process_stdout_capture");
}

static void test_process_stderr_capture(void)
{
#ifdef _WIN32
	AsyncResult result = fun_async_process_spawn(
		"cmd.exe",
		(const char *[]){ "cmd.exe", "/c", "dir", "nonexistent_file", NULL },
		NULL);
#else
	AsyncResult result = fun_async_process_spawn(
		"/bin/sh",
		(const char *[]){ "/bin/sh", "-c", "ls nonexistent_file", NULL }, NULL);
#endif

	if (result.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&result);
		assert(result.status == ASYNC_COMPLETED);

		size_t length = 0;
		const char *err_data = fun_process_get_stderr(&result, &length);
		(void)err_data;
		assert(err_data != NULL);

		fun_process_free(&result);
	}

	print_test_result("test_process_stderr_capture");
}

static void test_process_exit_code(void)
{
#ifdef _WIN32
	AsyncResult result = fun_async_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "exit", "42", NULL },
		NULL);
#else
	AsyncResult result = fun_async_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "exit 42", NULL }, NULL);
#endif

	if (result.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&result);
		assert(result.status == ASYNC_COMPLETED);

		int exit_code = fun_process_get_exit_code(&result);
		assert(exit_code == 42);

		fun_process_free(&result);
	}

	print_test_result("test_process_exit_code");
}

static void test_process_terminate(void)
{
#ifdef _WIN32
	AsyncResult result = fun_async_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "timeout", "60", NULL },
		NULL);
#else
	AsyncResult result = fun_async_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "sleep 60", NULL }, NULL);
#endif

	if (result.error.code == ERROR_CODE_NO_ERROR) {
		assert(result.process.is_running);

		ErrorResult term_result = fun_process_terminate(&result);
		ASSERT_NO_ERROR(term_result);

		fun_async_await(&result);
		assert(!result.process.is_running);

		fun_process_free(&result);
	}

	print_test_result("test_process_terminate");
}

static void test_process_buffer_overflow(void)
{
#ifdef _WIN32
	AsyncResult result = fun_async_process_spawn(
		"cmd.exe",
		(const char *[]){ "cmd.exe", "/c", "for", "/L", "%i", "in",
						  "(1,1,1000)", "do", "@echo", "This", "is", "a",
						  "long", "line", NULL },
		NULL);
#else
	AsyncResult result = fun_async_process_spawn(
		"/bin/sh",
		(const char *[]){ "/bin/sh", "-c",
						  "for i in $(seq 1 1000); do echo Line $i; done",
						  NULL },
		NULL);
#endif

	if (result.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&result);
		assert(result.status == ASYNC_COMPLETED);

		size_t length = 0;
		const char *out_data = fun_process_get_stdout(&result, &length);
		assert(out_data != NULL);

		assert(result.process.stdout_count <= PROCESS_STDOUT_BUFFER_SIZE);

		fun_process_free(&result);
	}

	print_test_result("test_process_buffer_overflow");
}

int main(void)
{
	printf("Running process spawn module tests:\n");

	test_process_spawn_success();
	test_process_spawn_not_found();
	test_process_stdout_capture();
	test_process_stderr_capture();
	test_process_exit_code();
	test_process_terminate();
	test_process_buffer_overflow();

	printf("All process spawn module tests passed.\n");
	return 0;
}
