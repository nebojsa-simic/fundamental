# Async Module Specification 
## Purpose  
Asynchronous operations module provides polling mechanism for non-blocking execution allowing applications to  
perform other work while waiting for operations.  

## Requirements 

### Requirement: Single operation await  
Async module SHALL provide function to suspend execution until single operation completes.  

#### Scenario: Wait for pending operation  
- **WHEN** fun_async_await is called with pending AsyncResult  
- **THEN** function blocks until operation completes or errors 

### Requirement: Multiple operation await  
Async module SHALL provide function to suspend execution until all operations in a collection complete.  

#### Scenario: Wait for multiple pending operations  
- **WHEN** fun_async_await_all is called with array of pending AsyncResult objects  
- **THEN** function blocks until all operations complete or error  
- **AND** each individual AsyncResult is polled independently during waiting  
- **AND** function returns when no operations remain pending  

## Constraints
- Functions MUST avoid busy-waiting when possible
- AsyncResult.status SHALL be updated during polling operations
- Poll function callbacks MUST be invoked to update operation status
