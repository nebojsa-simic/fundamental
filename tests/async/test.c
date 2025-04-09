#include <stdio.h>
#include <assert.h>
#include "../../include/async/async.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

// Helper function to check if an error occurred
#define ASSERT_NO_ERROR(result) assert(result.error.code == 0)
#define ASSERT_ERROR(result) assert(result.error.code != 0)

// Helper function to print test progress
void print_test_result(const char *test_name)
{
	printf("%s %s\n", GREEN_CHECK, test_name);
}

// -------------------------------------------------------------------------
// Dummy poll functions for testing

// This poll function simulates a successful asynchronous operation.
// It uses an integer counter (stored in result->state). For the first three
// polls, it returns ASYNC_PENDING; thereafter it returns ASYNC_COMPLETED.
static AsyncStatus test_poll_success(AsyncResult *result)
{
	int *counter = (int *)result->state;
	if (*counter < 3) {
		(*counter)++;
		return ASYNC_PENDING;
	}
	return ASYNC_COMPLETED;
}

// This poll function simulates an immediate error. It always returns ASYNC_ERROR.
static AsyncStatus test_poll_error_immediate(AsyncResult *result)
{
	return ASYNC_ERROR;
}

// This poll function simulates an asynchronous operation that errors after two polls.
// It uses an integer counter (stored in result->state). For the first two polls, it
// returns ASYNC_PENDING; thereafter it returns ASYNC_ERROR.
static AsyncStatus test_poll_error_after(AsyncResult *result)
{
	int *counter = (int *)result->state;
	if (*counter < 2) {
		(*counter)++;
		return ASYNC_PENDING;
	}
	return ASYNC_ERROR;
}

// -------------------------------------------------------------------------
// Unit tests for fun_async_await

// Test that fun_async_await correctly waits until an operation that eventually
// completes has finished.
static void test_fun_async_await_success(void)
{
	int counter = 0;
	AsyncResult result;
	result.poll = test_poll_success;
	result.state = &counter;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	fun_async_await(&result);
	// After enough polls, the operation should have completed.
	assert(result.status == ASYNC_COMPLETED);
	print_test_result("test_fun_async_await_success");
}

// Test that fun_async_await immediately handles an operation that errors.
static void test_fun_async_await_immediate_error(void)
{
	AsyncResult result;
	result.poll = test_poll_error_immediate;
	result.state = NULL;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	fun_async_await(&result);
	// The operation should immediately return with an error.
	assert(result.status == ASYNC_ERROR);
	print_test_result("test_fun_async_await_immediate_error");
}

// Test that fun_async_await correctly detects errors that occur after several polls.
static void test_fun_async_await_error_after(void)
{
	int counter = 0;
	AsyncResult result;
	result.poll = test_poll_error_after;
	result.state = &counter;
	result.status = ASYNC_PENDING;
	result.error = ERROR_RESULT_NO_ERROR;

	fun_async_await(&result);
	// After two polls the operation should error.
	assert(result.status == ASYNC_ERROR);
	print_test_result("test_fun_async_await_error_after");
}

// -------------------------------------------------------------------------
// Unit tests for fun_async_await_all

// Test fun_async_await_all with multiple successful operations.
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
	fun_async_await_all(results, 2);

	// Both operations should have completed.
	assert(result1.status == ASYNC_COMPLETED);
	assert(result2.status == ASYNC_COMPLETED);
	print_test_result("test_fun_async_await_all_success");
}

// Test fun_async_await_all with mixed operations: one that completes and one
// that immediately errors.
static void test_fun_async_await_all_mixed(void)
{
	int counter = 0;
	AsyncResult result1, result2;

	result1.poll = test_poll_success; // Will eventually complete.
	result1.state = &counter;
	result1.status = ASYNC_PENDING;
	result1.error = ERROR_RESULT_NO_ERROR;

	result2.poll = test_poll_error_immediate; // Immediately errors.
	result2.state = NULL;
	result2.status = ASYNC_PENDING;
	result2.error = ERROR_RESULT_NO_ERROR;

	AsyncResult *results[2] = { &result1, &result2 };
	fun_async_await_all(results, 2);

	// Expect that result1 is completed and result2 is in error state.
	assert(result1.status == ASYNC_COMPLETED);
	assert(result2.status == ASYNC_ERROR);
	print_test_result("test_fun_async_await_all_mixed");
}

// -------------------------------------------------------------------------
// Main entry point to run all async module tests.
int main(void)
{
	printf("Running async module tests:\n");
	
	test_fun_async_await_success();
	test_fun_async_await_immediate_error();
	test_fun_async_await_error_after();
	test_fun_async_await_all_success();
	test_fun_async_await_all_mixed();

	printf("All async module tests passed.\n");
	return 0;
}
