# Error Module Specification

## Purpose 
Error handling module provides standardized result types to represent success and error conditions across library components. 

## Requirements 

### Requirement: Standard Result Types
The error module SHALL provide standardized ErrorResult type with code and message fields.

#### Scenario: Create error with code and message
- **WHEN** fun_error_result(code, message) is called with specific parameters
- **THEN** function returns ErrorResult with provided code and message

### Requirement: Common Error Codes
The error module SHALL define standard error code constants.

#### Scenario: Access no error code
- **WHEN** ERROR_CODE_NO_ERROR is referenced
- **THEN** constant equals 0

#### Scenario: Access null pointer error code  
- **WHEN** ERROR_CODE_NULL_POINTER is referenced
- **THEN** constant equals 1

### Requirement: Error Check Functions
The error module SHALL provide functions to determine error states.

#### Scenario: Check error state
- **WHEN** fun_error_is_error(error) is called with error code != 0
- **THEN** function returns true
- **WHEN** fun_error_is_error(error) is called with error code == 0
- **THEN** function returns false

#### Scenario: Check OK state  
- **WHEN** fun_error_is_ok(error) is called with error code == 0
- **THEN** function returns true
- **WHEN** fun_error_is_ok(error) is called with error code != 0
- **THEN** function returns false

### Requirement: Result Type Generics  
The error module SHALL provide macro to create typed result types that include both values and errors.

#### Scenario: Use result type macro
- **WHEN** DEFINE_RESULT_TYPE(int) is used
- **THEN** intResult type is created containing int value and ErrorResult
- **AND** value and error can be accessed separately

## Constraints
- Error code 0 ALWAYS indicates no error condition
- ErrorResult SHALL include both code and message for debugging
- Non-zero error codes indicate failure conditions
