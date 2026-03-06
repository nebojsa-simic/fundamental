# Stream Module Specification  
  
## Scenarios 
  
Scenario: Open file read stream  
'  Given a file path and buffer'  
'  When fun_stream_create_file_read is called'  
'  Then FileStream for reading is returned' 
  
Scenario: Read from stream  
'  Given valid FileStream'  
'  When fun_stream_read is called'  
'  Then available data is read into buffer' 
