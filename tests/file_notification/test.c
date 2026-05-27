#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result)    assert(result.error.code != 0)

static void
test_notification_callback(String filePath)
{
	(void)filePath;
}

static bool
test_register_notification_success(void)
{
	AsyncResult result = fun_register_file_change_notification(
		"./test_notify_file.txt",
		test_notification_callback);

	bool success = (result.status == ASYNC_PENDING) &&
		(fun_error_is_ok(result.error)) &&
		(result.state != NULL);

	if (success) {
		fun_unregister_file_change_notification(result.state);
	}

	if (success)
		printf("✓ test_register_notification_success passed\n");
	return success;
}

static bool
test_register_notification_null_path(void)
{
	AsyncResult result = fun_register_file_change_notification(
		NULL, test_notification_callback);

	bool success = (result.status == ASYNC_ERROR) &&
		(fun_error_is_error(result.error));

	if (success)
		printf("✓ test_register_notification_null_path passed\n");
	return success;
}

static bool
test_register_notification_null_callback(void)
{
	AsyncResult result = fun_register_file_change_notification(
		"./test_notify_file.txt", NULL);

	bool success = (result.status == ASYNC_ERROR) &&
		(fun_error_is_error(result.error));

	if (success)
		printf("✓ test_register_notification_null_callback passed\n");
	return success;
}

static bool
test_unregister_notification_null_state(void)
{
	AsyncResult result =
		fun_unregister_file_change_notification(NULL);

	bool success = (result.status == ASYNC_ERROR) &&
		(fun_error_is_error(result.error));

	if (success)
		printf("✓ test_unregister_notification_null_state passed\n");
	return success;
}

static bool
test_register_and_unregister(void)
{
	AsyncResult reg = fun_register_file_change_notification(
		"./test_notify_file.txt",
		test_notification_callback);

	if (reg.status != ASYNC_PENDING || fun_error_is_error(reg.error)) {
		return false;
	}

	AsyncResult unreg =
		fun_unregister_file_change_notification(reg.state);

	bool success = (unreg.status == ASYNC_COMPLETED) &&
		(fun_error_is_ok(unreg.error));

	if (success)
		printf("✓ test_register_and_unregister passed\n");
	return success;
}

int main(void)
{
	FILE *fp = fopen("./test_notify_file.txt", "w");
	if (fp) {
		fprintf(fp, "notification test\n");
		fclose(fp);
	}

	printf("Running file notification module tests:\n");

	if (!test_register_notification_success()) {
		printf("Register notification test failed\n");
		remove("./test_notify_file.txt");
		return 1;
	}
	if (!test_register_notification_null_path()) {
		printf("Register null path test failed\n");
		remove("./test_notify_file.txt");
		return 1;
	}
	if (!test_register_notification_null_callback()) {
		printf("Register null callback test failed\n");
		remove("./test_notify_file.txt");
		return 1;
	}
	if (!test_unregister_notification_null_state()) {
		printf("Unregister null state test failed\n");
		remove("./test_notify_file.txt");
		return 1;
	}
	if (!test_register_and_unregister()) {
		printf("Register and unregister test failed\n");
		remove("./test_notify_file.txt");
		return 1;
	}

	remove("./test_notify_file.txt");
	printf("All file notification tests passed!\n");
	return 0;
}
