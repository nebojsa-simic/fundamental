#pragma once
#include "../error/error.h"

typedef enum {
    ASYNC_PENDING,
    ASYNC_COMPLETED,
    ASYNC_ERROR
} AsyncStatus;

typedef struct AsyncResult AsyncResult;
typedef AsyncStatus (*AsyncPollFn)(AsyncResult* result);

struct AsyncResult {
    AsyncPollFn poll;  // Poll function called repeatedly until the operation completes
    void* state;       // Opaque pointer to implementation-specific state
    AsyncStatus status;
    ErrorResult error;
};

void fun_async_await(AsyncResult* result);
void fun_async_await_all(AsyncResult** results, size_t count);
