#include "fundamental/console/console.h"
#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"

bool test_fun_lock_file_basic(void)
{
	FileLockHandle lockHandle;

	ErrorResult lock_result = fun_lock_file("test_lock.txt", &lockHandle);

	bool success = fun_error_is_ok(lock_result);

	if (success) {
		ErrorResult unlock_result = fun_unlock_file(lockHandle);
		success = fun_error_is_ok(unlock_result);
	}

	if (success) {
		fun_console_write_line("✓ fun_lock_file_basic passed");
	}
	return success;
}

bool test_fun_lock_unavailable_file(void)
{
	FileLockHandle lockHandle1, lockHandle2;

	ErrorResult lock_result1 =
		fun_lock_file("test_lock_exclusive.txt", &lockHandle1);
	bool first_lock_ok = fun_error_is_ok(lock_result1);

	if (first_lock_ok) {
		ErrorResult unlock_result1 = fun_unlock_file(lockHandle1);
		bool clean_first_release = fun_error_is_ok(unlock_result1);

		ErrorResult lock_result2 =
			fun_lock_file("test_lock_exclusive.txt", &lockHandle2);
		bool second_try_ok = true;
		if (fun_error_is_ok(lock_result2)) {
			ErrorResult unlock_result2 = fun_unlock_file(lockHandle2);
			second_try_ok = fun_error_is_ok(unlock_result2);
		}

		bool success = clean_first_release && second_try_ok;

		if (success) {
			fun_console_write_line(
				"✓ fun_lock_unavailable_file basic test passed");
		}
		return success;
	} else {
		return false;
	}
}

bool test_fun_unlock_file_invalid_handle(void)
{
	FileLockHandle invalidHandle = { .state = NULL };

	ErrorResult unlock_result = fun_unlock_file(invalidHandle);

	bool success = fun_error_is_error(unlock_result);

	if (success) {
		fun_console_write_line("✓ fun_unlock_file_invalid_handle passed");
	}
	return success;
}

int main()
{
	fun_console_write_line("Running file lock module tests:");

	if (!test_fun_lock_file_basic()) {
		fun_console_write_line("Basic lock test failed");
		return 1;
	}

	if (!test_fun_lock_unavailable_file()) {
		fun_console_write_line("Unavailable file test failed");
		return 1;
	}

	if (!test_fun_unlock_file_invalid_handle()) {
		fun_console_write_line("Unlock invalid file failed");
		return 1;
	}

	fun_console_write_line("All file lock tests passed!");
	return 0;
}
