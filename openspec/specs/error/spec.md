# Error Module Specification  
  
Provides universal error handling with result types.  
  
## Core Types  
  
- ErrorResult: code (uint8_t) + message (const char*)  
- DEFINE_RESULT_TYPE(T): macro for result types  
  
## Error Codes  
  
- 0: ERROR_CODE_NO_ERROR  
- 1: ERROR_CODE_NULL_POINTER  
  
## Helper Functions  
  
- fun_error_result: creates error result  
- fun_error_is_error: checks for error  
- fun_error_is_ok: checks for success 
