# String Module Specification  
  
## Scenarios 
  
Scenario: Convert integer to string  
'  Given integer value 42 and base 10'  
'  When fun_string_from_int(42, 10, output) is called'  
'  Then string "42" is placed in output buffer' 
  
Scenario: Get string length  
'  Given string "hello"'  
'  When fun_string_length("hello") is called'  
'  Then result equals 5' 
  
Scenario: Find substring  
'  Given strings "hello world" and "world"'  
'  When fun_string_index_of("hello world","world", 0) is called'  
'  Then result equals 6' 
