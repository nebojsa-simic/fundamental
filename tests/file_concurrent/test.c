#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/error/error.h"
#include "fundamental/async/async.h"
#include "fundamental/console/console.h"

static bool test_lock_file_basic(void)
{
	FileLockHandle handle = { .state = NULL };
	ErrorResult result = fun_lock_file("test_concurrent_lock.txt", &handle);

	bool success = fun_error_is_ok(result);
	if (success)
		fun_unlock_file(handle);

	if (success)
		fun_console_write_line("✓ test_lock_file_basic passed");
	return success;
}

static bool test_lock_file_with_timeout(void)
{
	FileLockHandle handle = { .state = NULL };
	ErrorResult result = fun_file_lock_with_timeout(
		"test_concurrent_timeout.txt", 2000, &handle);

	bool success = fun_error_is_ok(result);
	if (success)
		fun_unlock_file(handle);

	if (success)
		fun_console_write_line("✓ test_lock_file_with_timeout passed");
	return success;
}

static bool test_lock_already_locked_file(void)
{
	FileLockHandle handle1 = { .state = NULL };
	ErrorResult lock1 =
		fun_file_lock_with_timeout("test_concurrent_held.txt", 5000, &handle1);
	if (fun_error_is_error(lock1)) {
		return false;
	}

	FileLockHandle handle2 = { .state = NULL };
	ErrorResult lock2 =
		fun_file_lock_with_timeout("test_concurrent_held.txt", 200, &handle2);

	bool success = fun_error_is_error(lock2);

	fun_unlock_file(handle1);

	if (success)
		fun_console_write_line("✓ test_lock_already_locked_file passed");
	return success;
}

static bool test_lock_after_unlock(void)
{
	FileLockHandle handle1 = { .state = NULL };
	ErrorResult lock1 = fun_lock_file("test_concurrent_relock.txt", &handle1);
	if (fun_error_is_error(lock1)) {
		return false;
	}

	ErrorResult unlock = fun_unlock_file(handle1);
	if (fun_error_is_error(unlock)) {
		return false;
	}

	FileLockHandle handle2 = { .state = NULL };
	ErrorResult lock2 = fun_lock_file("test_concurrent_relock.txt", &handle2);

	bool success = fun_error_is_ok(lock2);
	if (success)
		fun_unlock_file(handle2);

	if (success)
		fun_console_write_line("✓ test_lock_after_unlock passed");
	return success;
}

static bool test_unlock_invalid_handle(void)
{
	FileLockHandle handle = { .state = NULL };
	ErrorResult result = fun_unlock_file(handle);

	bool success = fun_error_is_error(result);

	if (success)
		fun_console_write_line("✓ test_unlock_invalid_handle passed");
	return success;
}

static bool test_lock_file_null_path(void)
{
	FileLockHandle handle = { .state = NULL };
	ErrorResult result = fun_lock_file(NULL, &handle);

	bool success = fun_error_is_error(result);

	if (success)
		fun_console_write_line("✓ test_lock_file_null_path passed");
	return success;
}

static bool test_lock_file_null_handle(void)
{
	ErrorResult result = fun_lock_file("test_concurrent_null_handle.txt", NULL);

	bool success = fun_error_is_error(result);

	if (success)
		fun_console_write_line("✓ test_lock_file_null_handle passed");
	return success;
}

int main(void)
{
	fun_console_write_line("Running concurrent file tests:");

	if (!test_lock_file_basic()) {
		fun_console_write_line("Basic lock test failed");
		return 1;
	}
	if (!test_lock_file_with_timeout()) {
		fun_console_write_line("Lock with timeout test failed");
		return 1;
	}
	if (!test_lock_already_locked_file()) {
		fun_console_write_line("Already-locked file test failed");
		return 1;
	}
	if (!test_lock_after_unlock()) {
		fun_console_write_line("Lock after unlock test failed");
		return 1;
	}
	if (!test_unlock_invalid_handle()) {
		fun_console_write_line("Unlock invalid handle test failed");
		return 1;
	}
	if (!test_lock_file_null_path()) {
		fun_console_write_line("Lock null path test failed");
		return 1;
	}
	if (!test_lock_file_null_handle()) {
		fun_console_write_line("Lock null handle test failed");
		return 1;
	}

	fun_console_write_line("All concurrent file tests passed!");
	return 0;
}
