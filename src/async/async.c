#include "async.h"

void fun_async_await(AsyncResult* result) {
    // Continuously poll until the operation is no longer pending.
    while (result->status == ASYNC_PENDING) {
        // Call the poll function to update the status.
        AsyncStatus new_status = result->poll(result);
        result->status = new_status;

        // If an error has occurred, exit early.
        if (new_status == ASYNC_ERROR) {
            break;
        }
    }
}

void fun_async_await_all(AsyncResult** results, size_t count) {
    // Loop until every async result is no longer pending.
    while (1) {
        int stillPending = 0;
        for (size_t i = 0; i < count; i++) {
            if (results[i]->status == ASYNC_PENDING) {
                // Poll the pending async operation.
                AsyncStatus new_status = results[i]->poll(results[i]);
                results[i]->status = new_status;

                // Mark if at least one operation is still pending.
                if (new_status == ASYNC_PENDING) {
                    stillPending = 1;
                }
            }
        }

        // Exit once all operations are complete or have errored.
        if (!stillPending) {
            break;
        }
        
        // Optionally insert a platform-appropriate yield or sleep here to avoid busy-waiting.
    }
}
