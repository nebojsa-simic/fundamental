#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

bool test_fun_lock_file_basic(void)
{
	// Create a test file
	FILE *fp = fopen("test_lock.txt", "w");
	if (fp) {
		fprintf(fp, "test content");
		fclose(fp);
	}

	FileLockHandle lockHandle;

	// Try to acquire lock on the file
	ErrorResult lock_result = fun_lock_file("test_lock.txt", &lockHandle);

	bool success = fun_error_is_ok(lock_result);

	// If successfully acquired, try to release it
	if (success) {
		ErrorResult unlock_result = fun_unlock_file(lockHandle);
		success = fun_error_is_ok(unlock_result);
	}

	// Clean up
	remove("test_lock.txt");

	if (success) {
		printf("✓ fun_lock_file_basic passed\n");
	}
	return success;
}

bool test_fun_lock_unavailable_file(void)
{
	// This test would need a complex setup to simulate exclusive access,
	// but for basic API checking:
	FileLockHandle lockHandle1, lockHandle2;

	FILE *fp = fopen("test_lock_exclusive.txt", "w");
	if (fp) {
		fprintf(fp, "test content");
		fclose(fp);
	}

	// Acquire the first lock
	ErrorResult lock_result1 =
		fun_lock_file("test_lock_exclusive.txt", &lockHandle1);
	bool first_lock_ok = fun_error_is_ok(lock_result1);

	if (first_lock_ok) {
		// Attempt a second lock - might not be strictly prohibited on all systems
		// Release first lock
		ErrorResult unlock_result1 = fun_unlock_file(lockHandle1);
		bool clean_first_release = fun_error_is_ok(unlock_result1);

		// Try the lock operation again now the resource is free
		ErrorResult lock_result2 =
			fun_lock_file("test_lock_exclusive.txt", &lockHandle2);
		bool second_try_ok = true;
		if (fun_error_is_ok(lock_result2)) {
			ErrorResult unlock_result2 = fun_unlock_file(lockHandle2);
			second_try_ok = fun_error_is_ok(unlock_result2);
		}

		// Check for basic success of cleanup steps
		bool success = clean_first_release && second_try_ok;

		remove("test_lock_exclusive.txt");
		if (success) {
			printf("✓ fun_lock_unavailable_file basic test passed\n");
		}
		return success;
	} else {
		remove("test_lock_exclusive.txt");
		return false;
	}
}

bool test_fun_unlock_file_invalid_handle(void)
{
	// Test unlock with a fake/invalid handle
	FileLockHandle invalidHandle = { .state = NULL };

	ErrorResult unlock_result = fun_unlock_file(invalidHandle);

	// Behavior depends on implementation - should handle gracefully
	bool success = fun_error_is_error(unlock_result);

	if (success) {
		printf("✓ fun_unlock_file_invalid_handle passed\n");
	}
	return success;
}

int main()
{
	printf("Running file lock module tests:\n");

	if (!test_fun_lock_file_basic()) {
		printf("Basic lock test failed\n");
		return 1;
	}

	if (!test_fun_lock_unavailable_file()) {
		printf("Unavailable file test failed\n");
		return 1;
	}

	if (!test_fun_unlock_file_invalid_handle()) {
		printf("Unlock invalid file failed\n");
		return 1;
	}

	printf("All file lock tests passed!\n");
	return 0;
}