#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/error/error.h"
#include "fundamental/async/async.h"

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result)    assert(result.error.code != 0)

static bool
test_lock_file_basic(void)
{
	FILE *fp = fopen("test_concurrent_lock.txt", "w");
	if (!fp)
		return false;
	fclose(fp);

	FileLockHandle handle = { .state = NULL };
	ErrorResult result = fun_lock_file("test_concurrent_lock.txt", &handle);

	bool success = fun_error_is_ok(result);
	if (success)
		fun_unlock_file(handle);

	remove("test_concurrent_lock.txt");

	if (success)
		printf("✓ test_lock_file_basic passed\n");
	return success;
}

static bool
test_lock_file_with_timeout(void)
{
	FILE *fp = fopen("test_concurrent_timeout.txt", "w");
	if (!fp)
		return false;
	fclose(fp);

	FileLockHandle handle = { .state = NULL };
	ErrorResult result =
		fun_file_lock_with_timeout("test_concurrent_timeout.txt", 2000,
					   &handle);

	bool success = fun_error_is_ok(result);
	if (success)
		fun_unlock_file(handle);

	remove("test_concurrent_timeout.txt");

	if (success)
		printf("✓ test_lock_file_with_timeout passed\n");
	return success;
}

static bool
test_lock_already_locked_file(void)
{
	FILE *fp = fopen("test_concurrent_held.txt", "w");
	if (!fp)
		return false;
	fclose(fp);

	FileLockHandle handle1 = { .state = NULL };
	ErrorResult lock1 =
		fun_file_lock_with_timeout("test_concurrent_held.txt", 5000,
					   &handle1);
	if (fun_error_is_error(lock1)) {
		remove("test_concurrent_held.txt");
		return false;
	}

	FileLockHandle handle2 = { .state = NULL };
	ErrorResult lock2 =
		fun_file_lock_with_timeout("test_concurrent_held.txt", 200,
					   &handle2);

	bool success = fun_error_is_error(lock2);

	fun_unlock_file(handle1);
	remove("test_concurrent_held.txt");

	if (success)
		printf("✓ test_lock_already_locked_file passed\n");
	return success;
}

static bool
test_lock_after_unlock(void)
{
	FILE *fp = fopen("test_concurrent_relock.txt", "w");
	if (!fp)
		return false;
	fclose(fp);

	FileLockHandle handle1 = { .state = NULL };
	ErrorResult lock1 = fun_lock_file("test_concurrent_relock.txt", &handle1);
	if (fun_error_is_error(lock1)) {
		remove("test_concurrent_relock.txt");
		return false;
	}

	ErrorResult unlock = fun_unlock_file(handle1);
	if (fun_error_is_error(unlock)) {
		remove("test_concurrent_relock.txt");
		return false;
	}

	FileLockHandle handle2 = { .state = NULL };
	ErrorResult lock2 = fun_lock_file("test_concurrent_relock.txt", &handle2);

	bool success = fun_error_is_ok(lock2);
	if (success)
		fun_unlock_file(handle2);

	remove("test_concurrent_relock.txt");

	if (success)
		printf("✓ test_lock_after_unlock passed\n");
	return success;
}

static bool
test_unlock_invalid_handle(void)
{
	FileLockHandle handle = { .state = NULL };
	ErrorResult result = fun_unlock_file(handle);

	bool success = fun_error_is_error(result);

	if (success)
		printf("✓ test_unlock_invalid_handle passed\n");
	return success;
}

static bool
test_lock_file_null_path(void)
{
	FileLockHandle handle = { .state = NULL };
	ErrorResult result = fun_lock_file(NULL, &handle);

	bool success = fun_error_is_error(result);

	if (success)
		printf("✓ test_lock_file_null_path passed\n");
	return success;
}

static bool
test_lock_file_null_handle(void)
{
	FILE *fp = fopen("test_concurrent_null_handle.txt", "w");
	if (!fp)
		return false;
	fclose(fp);

	ErrorResult result = fun_lock_file("test_concurrent_null_handle.txt", NULL);

	bool success = fun_error_is_error(result);

	remove("test_concurrent_null_handle.txt");

	if (success)
		printf("✓ test_lock_file_null_handle passed\n");
	return success;
}

int main(void)
{
	printf("Running concurrent file tests:\n");

	if (!test_lock_file_basic()) {
		printf("Basic lock test failed\n");
		return 1;
	}
	if (!test_lock_file_with_timeout()) {
		printf("Lock with timeout test failed\n");
		return 1;
	}
	if (!test_lock_already_locked_file()) {
		printf("Already-locked file test failed\n");
		return 1;
	}
	if (!test_lock_after_unlock()) {
		printf("Lock after unlock test failed\n");
		return 1;
	}
	if (!test_unlock_invalid_handle()) {
		printf("Unlock invalid handle test failed\n");
		return 1;
	}
	if (!test_lock_file_null_path()) {
		printf("Lock null path test failed\n");
		return 1;
	}
	if (!test_lock_file_null_handle()) {
		printf("Lock null handle test failed\n");
		return 1;
	}

	printf("All concurrent file tests passed!\n");
	return 0;
}
