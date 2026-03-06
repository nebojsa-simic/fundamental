# Memory Module Specification  
  
Provides platform-independent memory management.  
  
## Design Pattern  
  
Caller-allocated: functions don't allocate internally.  
  
## Core Functions  
  
- fun_memory_allocate: allocates memory  
- fun_memory_reallocate: reallocates memory  
- fun_memory_free: frees memory  
- fun_memory_copy: copies memory  
- fun_memory_fill: fills memory with value  
