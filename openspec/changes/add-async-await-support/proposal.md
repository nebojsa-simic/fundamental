## Why

Developers building async applications must manually poll `AsyncResult` objects or block with `fun_async_await()`. This makes it difficult to write efficient concurrent code that handles thousands of connections without blocking threads.

## What Changes

- Introduce native async/await syntax support through the compiler
- Add `await` keyword for non-blocking suspension points
- Add `async` function markers for suspendable functions
- **BREAKING**: Existing manual polling patterns will be deprecated but supported during migration

## Capabilities

### New Capabilities
- `async-await-syntax`: Language-level async/await support with suspension and resumption
- `async-runtime`: Event loop and task scheduler for executing async functions
- `async-combinators`: Utilities like `race()`, `join()`, `select()` for composing async operations

### Modified Capabilities
- `network`: Network operations will gain async-first interfaces

## Impact

- Requires compiler modifications to support `async`/`await` keywords
- Existing `AsyncResult` pattern remains for FFI compatibility
- All network and I/O operations become truly non-blocking
- Enables efficient handling of 10,000+ concurrent connections
