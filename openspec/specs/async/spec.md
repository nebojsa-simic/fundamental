# Async Module Specification  
  
Provides async operations with polling.  
  
## Core Types  
  
- AsyncResult: poll function + state + status + error  
- AsyncStatus: PENDING, COMPLETED, ERROR  
  
## Core Functions  
  
- fun_async_await: blocks until operation completes  
- fun_async_await_all: waits for multiple operations  
  
## Design Pattern  
  
- Polling-based async operations  
- State management for async operations 
