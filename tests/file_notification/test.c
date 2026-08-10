#include "fundamental/console/console.h"
#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/async/async.h"

#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

static void test_notification_callback(String filePath)
{
	(void)filePath;
}

static bool test_register_notification_success(void)
{
	AsyncResult result = fun_register_file_change_notification(
		"./test_notify_file.txt", test_notification_callback);

	bool success = (result.status == ASYNC_PENDING) &&
				   (fun_error_is_ok(result.error)) && (result.state != NULL);

	if (success) {
		fun_unregister_file_change_notification(result.state);
	}

	if (success)
		fun_console_write_line("✓ test_register_notification_success passed");
	return success;
}

static bool test_register_notification_null_path(void)
{
	AsyncResult result =
		fun_register_file_change_notification(NULL, test_notification_callback);

	bool success = (result.status == ASYNC_ERROR) &&
				   (fun_error_is_error(result.error));

	if (success)
		fun_console_write_line("✓ test_register_notification_null_path passed");
	return success;
}

static bool test_register_notification_null_callback(void)
{
	AsyncResult result =
		fun_register_file_change_notification("./test_notify_file.txt", NULL);

	bool success = (result.status == ASYNC_ERROR) &&
				   (fun_error_is_error(result.error));

	if (success)
		fun_console_write_line(
			"✓ test_register_notification_null_callback passed");
	return success;
}

static bool test_unregister_notification_null_state(void)
{
	AsyncResult result = fun_unregister_file_change_notification(NULL);

	bool success = (result.status == ASYNC_ERROR) &&
				   (fun_error_is_error(result.error));

	if (success)
		fun_console_write_line(
			"✓ test_unregister_notification_null_state passed");
	return success;
}

static bool test_register_and_unregister(void)
{
	AsyncResult reg = fun_register_file_change_notification(
		"./test_notify_file.txt", test_notification_callback);

	if (reg.status != ASYNC_PENDING || fun_error_is_error(reg.error)) {
		return false;
	}

	AsyncResult unreg = fun_unregister_file_change_notification(reg.state);

	bool success = (unreg.status == ASYNC_COMPLETED) &&
				   (fun_error_is_ok(unreg.error));

	if (success)
		fun_console_write_line("✓ test_register_and_unregister passed");
	return success;
}

int main(void)
{
	fun_console_write_line("Running file notification module tests:");

	if (!test_register_notification_success()) {
		fun_console_write_line("Register notification test failed");
		return 1;
	}
	if (!test_register_notification_null_path()) {
		fun_console_write_line("Register null path test failed");
		return 1;
	}
	if (!test_register_notification_null_callback()) {
		fun_console_write_line("Register null callback test failed");
		return 1;
	}
	if (!test_unregister_notification_null_state()) {
		fun_console_write_line("Unregister null state test failed");
		return 1;
	}
	if (!test_register_and_unregister()) {
		fun_console_write_line("Register and unregister test failed");
		return 1;
	}

	fun_console_write_line("All file notification tests passed!");
	return 0;
}
