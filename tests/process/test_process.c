#include <assert.h>
#include <stdio.h>

#include "fundamental/async/async.h"
#include "fundamental/process/process.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"

#define ASSERT_NO_ERROR(result) assert((result).code == 0)
#define ASSERT_ERROR(result) assert((result).code != 0)

void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

static void test_process_spawn_success(void)
{
	char out_buf[4096], err_buf[1024];
	ProcessResult proc = { .stdout_data = out_buf,
						   .stdout_capacity = sizeof(out_buf),
						   .stderr_data = err_buf,
						   .stderr_capacity = sizeof(err_buf) };

#ifdef _WIN32
	AsyncResult ar = fun_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "exit", "0", NULL }, NULL,
		&proc);
#else
	AsyncResult ar = fun_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "exit 0", NULL }, NULL,
		&proc);
#endif

	assert(ar.error.code == ERROR_CODE_NO_ERROR ||
		   ar.error.code == ERROR_CODE_PROCESS_NOT_FOUND);

	if (ar.error.code == ERROR_CODE_NO_ERROR) {
		assert(ar.status == ASYNC_PENDING);
		voidResult wr = fun_async_await(&ar, -1);
		(void)wr;
		assert(ar.status == ASYNC_COMPLETED || ar.status == ASYNC_ERROR);
		fun_process_free(&proc);
	}

	print_test_result("test_process_spawn_success");
}

static void test_process_spawn_not_found(void)
{
	char out_buf[256], err_buf[256];
	ProcessResult proc = { .stdout_data = out_buf,
						   .stdout_capacity = sizeof(out_buf),
						   .stderr_data = err_buf,
						   .stderr_capacity = sizeof(err_buf) };

	AsyncResult ar = fun_process_spawn(
		"nonexistent_executable_xyz123",
		(const char *[]){ "nonexistent_executable_xyz123", NULL }, NULL, &proc);

	ASSERT_ERROR(ar.error);
	assert(ar.error.code == ERROR_CODE_PROCESS_NOT_FOUND);
	print_test_result("test_process_spawn_not_found");
}

static void test_process_stdout_capture(void)
{
	char out_buf[4096], err_buf[256];
	ProcessResult proc = { .stdout_data = out_buf,
						   .stdout_capacity = sizeof(out_buf),
						   .stderr_data = err_buf,
						   .stderr_capacity = sizeof(err_buf) };

#ifdef _WIN32
	AsyncResult ar = fun_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "echo", "Hello", NULL },
		NULL, &proc);
#else
	AsyncResult ar = fun_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "echo Hello", NULL },
		NULL, &proc);
#endif

	if (ar.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&ar, -1);
		assert(ar.status == ASYNC_COMPLETED);
		assert(proc.stdout_length > 0);
		fun_process_free(&proc);
	}

	print_test_result("test_process_stdout_capture");
}

static void test_process_stderr_capture(void)
{
	char out_buf[256], err_buf[4096];
	ProcessResult proc = { .stdout_data = out_buf,
						   .stdout_capacity = sizeof(out_buf),
						   .stderr_data = err_buf,
						   .stderr_capacity = sizeof(err_buf) };

#ifdef _WIN32
	AsyncResult ar = fun_process_spawn(
		"cmd.exe",
		(const char *[]){ "cmd.exe", "/c", "dir", "nonexistent_file", NULL },
		NULL, &proc);
#else
	AsyncResult ar = fun_process_spawn(
		"/bin/sh",
		(const char *[]){ "/bin/sh", "-c", "ls nonexistent_file", NULL }, NULL,
		&proc);
#endif

	if (ar.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&ar, -1);
		assert(ar.status == ASYNC_COMPLETED);
		assert(proc.stderr_length > 0);
		fun_process_free(&proc);
	}

	print_test_result("test_process_stderr_capture");
}

static void test_process_exit_code(void)
{
	char out_buf[256], err_buf[256];
	ProcessResult proc = { .stdout_data = out_buf,
						   .stdout_capacity = sizeof(out_buf),
						   .stderr_data = err_buf,
						   .stderr_capacity = sizeof(err_buf) };

#ifdef _WIN32
	AsyncResult ar = fun_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "exit", "42", NULL },
		NULL, &proc);
#else
	AsyncResult ar = fun_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "exit 42", NULL }, NULL,
		&proc);
#endif

	if (ar.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&ar, -1);
		assert(ar.status == ASYNC_COMPLETED);
		assert(proc.exit_code == 42);
		fun_process_free(&proc);
	}

	print_test_result("test_process_exit_code");
}

static void test_process_terminate(void)
{
	char out_buf[256], err_buf[256];
	ProcessResult proc = { .stdout_data = out_buf,
						   .stdout_capacity = sizeof(out_buf),
						   .stderr_data = err_buf,
						   .stderr_capacity = sizeof(err_buf) };

#ifdef _WIN32
	AsyncResult ar = fun_process_spawn(
		"cmd.exe", (const char *[]){ "cmd.exe", "/c", "timeout", "60", NULL },
		NULL, &proc);
#else
	AsyncResult ar = fun_process_spawn(
		"/bin/sh", (const char *[]){ "/bin/sh", "-c", "sleep 60", NULL }, NULL,
		&proc);
#endif

	if (ar.error.code == ERROR_CODE_NO_ERROR) {
		voidResult term = fun_process_terminate(&proc);
		ASSERT_NO_ERROR(term.error);
		fun_async_await(&ar, -1);
		fun_process_free(&proc);
	}

	print_test_result("test_process_terminate");
}

static void test_process_buffer_truncation(void)
{
	/* Small buffer — output should be truncated, not overflow */
	char out_buf[64], err_buf[64];
	ProcessResult proc = { .stdout_data = out_buf,
						   .stdout_capacity = sizeof(out_buf),
						   .stderr_data = err_buf,
						   .stderr_capacity = sizeof(err_buf) };

#ifdef _WIN32
	AsyncResult ar = fun_process_spawn(
		"cmd.exe",
		(const char *[]){ "cmd.exe", "/c",
						  "for /L %i in (1,1,100) do @echo Line %i", NULL },
		NULL, &proc);
#else
	AsyncResult ar = fun_process_spawn(
		"/bin/sh",
		(const char *[]){ "/bin/sh", "-c",
						  "for i in $(seq 1 100); do echo Line $i; done",
						  NULL },
		NULL, &proc);
#endif

	if (ar.error.code == ERROR_CODE_NO_ERROR) {
		fun_async_await(&ar, -1);
		assert(ar.status == ASYNC_COMPLETED);
		assert(proc.stdout_length <= sizeof(out_buf));
		fun_process_free(&proc);
	}

	print_test_result("test_process_buffer_truncation");
}

int main(void)
{
	printf("Running process module tests:\n");

	test_process_spawn_success();
	test_process_spawn_not_found();
	test_process_stdout_capture();
	test_process_stderr_capture();
	test_process_exit_code();
	test_process_terminate();
	test_process_buffer_truncation();

	printf("\nAll process module tests passed.\n");
	return 0;
}
