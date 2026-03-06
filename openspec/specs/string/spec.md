# String Module Specification

## Purpose
String module provides safe string manipulation operations with type conversions, comparisons, and template handling for formatting.

## Requirements

### Requirement: Conversion Operations
The string module SHALL provide functions to convert various data types to string representations.

#### Scenario: Convert integer to string
- **WHEN** fun_string_from_int(value, base, output) is called with valid parameters
- **THEN** string representation of integer is placed in output buffer
- **AND** base between 2-36 is correctly used for representation

#### Scenario: Convert double to string
- **WHEN** fun_string_from_double(number, decimal_places, output) is called
- **THEN** number is represented with specified decimal places
- **AND** result stored in output buffer

#### Scenario: Convert pointer to string
- **WHEN** fun_string_from_pointer(ptr, output) is called
- **THEN** hexadecimal representation of pointer address is stored

### Requirement: Validation Operations
The string module SHALL provide validation for string integrity and safety properties.

#### Scenario: Validate string safety
- **WHEN** fun_string_is_valid(string, max_length) is called with valid string
- **THEN** function verifies null termination
- **AND** no more than max_length characters before null termination
- **IF** string is NULL
- **THEN** function returns ERROR_RESULT_NULL_POINTER

### Requirement: Comparison and Search Operations
The string module SHALL provide operations to compare and search for string content.

#### Scenario: Compare two strings
- **WHEN** fun_string_compare(str1, str2) is called
- **THEN** function returns zero if equal
- **AND** returns positive if str1 > str2, negative if str1 < str2

#### Scenario: Find substring
- **WHEN** fun_string_index_of(haystack, needle, start_pos) is called
- **THEN** function returns position of needle in haystack starting from position
- **IF** substring not found
- **THEN** function returns -1

### Requirement: Size and Position Operations
The string module SHALL provide operations to retrieve string characteristics.

#### Scenario: Get string length
- **WHEN** fun_string_length(string) is called
- **THEN** function returns character count up to null terminator

### Requirement: In-Place String Manipulation
The string module SHALL provide operations that modify strings without creating new memory.

#### Scenario: Trim whitespace from string
- **WHEN** fun_string_trim_in_place(input) is called
- **THEN** leading and trailing whitespace is removed in place

#### Scenario: Reverse string in place
- **WHEN** fun_string_reverse_in_place(input) is called  
- **THEN** characters in string are reversed in place

### Requirement: Out-of-Place String Operations
The string module SHALL provide operations that generate new string values.

#### Scenario: Join two strings
- **WHEN** fun_string_join(left, right, output) is called
- **THEN** concatenated result is placed in output buffer

#### Scenario: Copy string
- **WHEN** fun_string_copy(source, output) is called with valid strings
- **THEN** source is cloned to output buffer

### Requirement: String Template Operations  
The string module SHALL provide template-based string substitution.

#### Scenario: Apply template with variable substitution
- **WHEN** fun_string_template(template, params, count, output) is called
- **THEN** placeholders like $(var), #{int}, %(double), *(ptr) are substituted
- **AND** result is written to output buffer

## Constraints
- Output buffers must have adequate space to store results
- Functions shall handle NULL parameter appropriately
- Template placeholders shall follow specific prefixes: $, #, %, *
- String operations must not access memory beyond bounds
