# Memory Module Specification  
  
## Scenarios  
  
### Allocate Memory  
  
When fun_memory_allocate(1024) is called  
Then returns MemoryResult with allocated buffer  
  
### Free Memory  
  
When fun_memory_free is called with valid buffer  
Then memory is deallocated 
