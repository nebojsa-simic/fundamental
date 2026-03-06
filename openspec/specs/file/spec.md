# File Module Specification  
  
## Scenarios 
  
Scenario: Read file into memory  
'  Given valid file path and output buffer'  
'  When fun_read_file_in_memory is called'  
'  Then file contents are read into buffer asynchronously' 
  
Scenario: Write memory to file  
'  Given input data and valid file path'  
'  When fun_write_memory_to_file is called'  
'  Then data is written to file asynchronously' 
