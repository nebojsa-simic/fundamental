#include "fundamental/async/async.h"
#include "fundamental/console/console.h"

#define GREEN_CHECK "\033[0;32m\342\234\223\033[0m"

void print_test_result(const char *test_name)
{
	fun_console_write(GREEN_CHECK);
	fun_console_write(" ");
	fun_console_write_line(test_name);
}

/* -------------------------------------------------------------------------
 * Dummy poll functions for testing
 */

/* Succeeds after 3 polls */
static AsyncStatus test_poll_success(AsyncResult *result)
{
	int *counter = (int *)result->state;
	if (*counter < 3) {
		(*counter)++;
		return ASYNC_PENDING;
	}
	result->status = ASYNC_COMPLETED;
	return ASYNC_COMPLETED;
}

/* Errors immediately */
static AsyncStatus test_poll_error_immediate(AsyncResult *result)
{
	(void)result;
	return ASYNC_ERROR;
}

/* Errors after 2 polls */
static AsyncStatus test_poll_error_after(AsyncResult *result)
{
	int *counter = (int *)result->state;
	if (*counter < 2) {
		(*counter)++;
		return ASYNC_PENDING;
	}
	return ASYNC_ERROR;
}

/* Always stays pending — used for timeout tests */
static AsyncStatus test_poll_always_pending(AsyncResult *result)
{
	(void)result;
	return ASYNC_PENDING;
}

/* -------------------------------------------------------------------------
 * Unit tests for fun_async_await
 */

static void test_fun_async_await_success(void)
{
	int counter = 0;
	AsyncResult result;
	result.poll = test_poll_success;
	result.state = &counter;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	voidResult wr = fun_async_await(&result, -1);
	(void)wr;
	if (!(result.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	print_test_result("test_fun_async_await_success");
}

static void test_fun_async_await_immediate_error(void)
{
	AsyncResult result;
	result.poll = test_poll_error_immediate;
	result.state = NULL;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	voidResult wr = fun_async_await(&result, -1);
	(void)wr;
	if (!(result.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	print_test_result("test_fun_async_await_immediate_error");
}

static void test_fun_async_await_error_after(void)
{
	int counter = 0;
	AsyncResult result;
	result.poll = test_poll_error_after;
	result.state = &counter;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	voidResult wr = fun_async_await(&result, -1);
	(void)wr;
	if (!(result.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	print_test_result("test_fun_async_await_error_after");
}

/* timeout_ms=0: single poll on an incomplete op → ERROR_CODE_ASYNC_TIMEOUT */
static void test_fun_async_await_timeout_zero(void)
{
	AsyncResult result;
	result.poll = test_poll_always_pending;
	result.state = NULL;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	voidResult wr = fun_async_await(&result, 0);
	if (wr.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
	if (!(wr.error.code == ERROR_CODE_ASYNC_TIMEOUT)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(result.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	print_test_result("test_fun_async_await_timeout_zero");
}

/* timeout_ms>0 that expires → ERROR_CODE_ASYNC_TIMEOUT */
static void test_fun_async_await_timeout_expires(void)
{
	AsyncResult result;
	result.poll = test_poll_always_pending;
	result.state = NULL;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	voidResult wr = fun_async_await(&result, 50);
	if (wr.error.code == 0) {
		fun_console_write_line("FAIL: ASSERT_ERROR");
		return;
	}
	if (!(wr.error.code == ERROR_CODE_ASYNC_TIMEOUT)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(result.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	print_test_result("test_fun_async_await_timeout_expires");
}

/* -------------------------------------------------------------------------
 * Unit tests for fun_async_await_all
 */

static void test_fun_async_await_all_success(void)
{
	int counter1 = 0, counter2 = 0;
	AsyncResult result1, result2;

	result1.poll = test_poll_success;
	result1.state = &counter1;
	result1.status = ASYNC_PENDING;
	result1.error = ERROR_RESULT_NO_ERROR;

	result2.poll = test_poll_success;
	result2.state = &counter2;
	result2.status = ASYNC_PENDING;
	result2.error = ERROR_RESULT_NO_ERROR;

	AsyncResult *results[2] = { &result1, &result2 };
	voidResult wr = fun_async_await_all(results, 2, -1);
	(void)wr;

	if (!(result1.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(result2.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	print_test_result("test_fun_async_await_all_success");
}

static void test_fun_async_await_all_mixed(void)
{
	int counter = 0;
	AsyncResult result1, result2;

	result1.poll = test_poll_success;
	result1.state = &counter;
	result1.status = ASYNC_PENDING;
	result1.error = ERROR_RESULT_NO_ERROR;

	result2.poll = test_poll_error_immediate;
	result2.state = NULL;
	result2.status = ASYNC_PENDING;
	result2.error = ERROR_RESULT_NO_ERROR;

	AsyncResult *results[2] = { &result1, &result2 };
	voidResult wr = fun_async_await_all(results, 2, -1);
	(void)wr;

	if (!(result1.status == ASYNC_COMPLETED)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	if (!(result2.status == ASYNC_ERROR)) {
		fun_console_write_line("FAIL: assertion");
		return;
	}
	print_test_result("test_fun_async_await_all_mixed");
}

/* -------------------------------------------------------------------------
 * Main
 */
int main(void)
{
	fun_console_write_line("Running async module tests:");

	test_fun_async_await_success();
	test_fun_async_await_immediate_error();
	test_fun_async_await_error_after();
	test_fun_async_await_timeout_zero();
	test_fun_async_await_timeout_expires();
	test_fun_async_await_all_success();
	test_fun_async_await_all_mixed();

	fun_console_write_line("");
	fun_console_write_line("All async module tests passed.");
	return 0;
}
