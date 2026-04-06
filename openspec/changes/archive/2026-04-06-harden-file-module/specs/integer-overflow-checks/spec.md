# Integer Overflow Checks

## Summary

Add overflow validation to all arithmetic operations involving sizes, offsets, and counts to prevent buffer overflows and memory corruption.

## ADDED Requirements

### Requirement: Overflow Check for Addition

All addition operations involving sizes and offsets SHALL check for overflow.

#### Scenario: offset + bytes_to_read overflow
- **WHEN** calculating `offset + bytes_to_read`
- **THEN** overflow is detected using `a > UINT64_MAX - b`
- **THEN** operation returns `ASYNC_ERROR` with `ERROR_RESULT_INTEGER_OVERFLOW`

#### Scenario: current_size + bytes_to_write overflow
- **WHEN** calculating `current_size + bytes_to_write`
- **THEN** overflow is detected before addition
- **THEN** operation returns error if overflow would occur

### Requirement: Overflow Check for Multiplication

Multiplication operations SHALL check for overflow.

#### Scenario: view size calculation
- **WHEN** calculating view size as multiplication
- **THEN** overflow is detected using `a > UINT64_MAX / b`
- **THEN** operation returns error if overflow detected

### Requirement: Portable Overflow Detection

Overflow detection SHALL use portable C code without compiler builtins.

#### Scenario: Portable add check
- **WHEN** checking `would_overflow_add(a, b)`
- **THEN** implementation uses `a > UINT64_MAX - b`
- **THEN** works on all C17 compilers

#### Scenario: Portable mul check
- **WHEN** checking `would_overflow_mul(a, b)`
- **THEN** implementation uses `a > UINT64_MAX / b` (for non-zero)
- **THEN** works on all C17 compilers

### Requirement: Error Code Definition

The error code `ERROR_RESULT_INTEGER_OVERFLOW` SHALL be defined.

#### Scenario: Overflow error returned
- **WHEN** overflow is detected
- **THEN** async result status is `ASYNC_ERROR`
- **THEN** error code is `ERROR_RESULT_INTEGER_OVERFLOW`

## Implementation

### Portable Overflow Check

```c
// include/file_overflow.h
static inline bool fun_would_overflow_add(uint64_t a, uint64_t b) {
    return a > UINT64_MAX - b;
}

static inline bool fun_would_overflow_mul(uint64_t a, uint64_t b) {
    if (a == 0 || b == 0) return false;
    return a > UINT64_MAX / b;
}
```

### Usage in Read Operations

```c
// src/fileReadMmap.c
uint64_t end_offset;
if (fun_would_overflow_add(state->parameters.offset, 
                           state->parameters.bytes_to_read)) {
    return ERROR_RESULT_INTEGER_OVERFLOW;
}
end_offset = state->parameters.offset + state->parameters.bytes_to_read;

uint64_t view_size;
if (fun_would_overflow_add(state->parameters.bytes_to_read,
                           state->parameters.offset - state->adjusted_offset)) {
    return ERROR_RESULT_INTEGER_OVERFLOW;
}
view_size = state->parameters.bytes_to_read + 
            (state->parameters.offset - state->adjusted_offset);
```

### Usage in Write Operations

```c
// src/fileWriteMmap.c
uint64_t required_size;
if (fun_would_overflow_add(file_stat.st_size, 
                           state->parameters.bytes_to_write)) {
    return ERROR_RESULT_INTEGER_OVERFLOW;
}
required_size = file_stat.st_size + state->parameters.bytes_to_write;

uint64_t view_size;
if (fun_would_overflow_add(state->parameters.bytes_to_write,
                           state->parameters.offset - state->adjusted_offset)) {
    return ERROR_RESULT_INTEGER_OVERFLOW;
}
view_size = state->parameters.bytes_to_write + 
            (state->parameters.offset - state->adjusted_offset);
```

## Testing

```c
// tests/file_overflow/test_file_overflow.c
void test_read_overflow_near_max(void) {
    Memory buffer = fun_memory_alloc(1024);
    FileReadParameters params = {
        .file_path = fun_string_create("/tmp/test.bin"),
        .output = buffer,
        .offset = UINT64_MAX - 100,
        .bytes_to_read = 200  // Would overflow
    };
    
    AsyncResult result = fun_file_read(params);
    assert(result.status == ASYNC_ERROR);
    assert(result.error.code == ERROR_RESULT_INTEGER_OVERFLOW);
}

void test_write_overflow_near_max(void) {
    FileWriteParameters params = {
        .file_path = fun_string_create("/tmp/test.bin"),
        .offset = UINT64_MAX - 100,
        .bytes_to_write = 200  // Would overflow
    };
    
    AsyncResult result = fun_file_write(params);
    assert(result.status == ASYNC_ERROR);
    assert(result.error.code == ERROR_RESULT_INTEGER_OVERFLOW);
}
```

## Error Codes

Add to `include/file_error.h`:
```c
#define ERROR_RESULT_INTEGER_OVERFLOW ((uint32_t)0x000000XX)
```

## Files Modified

- `include/file_overflow.h` (new)
- `src/fileRead.c`
- `src/fileReadMmap.c`
- `src/fileReadRing.c`
- `src/fileWrite.c`
- `src/fileWriteMmap.c`
- `src/fileWriteRing.c`
- `src/fileAppend.c`
- `include/file_error.h`
- `tests/file_overflow/test_file_overflow.c` (new)
