# Error Module Specification  
  
## Scenarios 
  
Scenario: Create error result with code and message  
'  Given a code and message'  
'  When fun_error_result(1, "Null pointer provided") is called'  
'  Then ErrorResult with matching code and message is returned' 
  
Scenario: Check for existing error  
'  Given ErrorResult with code equals 1'  
'  When fun_error_is_error is called'  
'  Then result equals true' 
  
Scenario: Check for no error case  
'  Given ErrorResult with code equals 0'  
'  When fun_error_is_error is called'  
'  Then result equals false' 
  
Scenario: Check for success case  
'  Given ErrorResult with code equals 0'  
'  When fun_error_is_ok is called'  
'  Then result equals true' 
  
Scenario: Check for failure case  
'  Given ErrorResult with code equals 1'  
'  When fun_error_is_ok is called'  
'  Then result equals false' 
