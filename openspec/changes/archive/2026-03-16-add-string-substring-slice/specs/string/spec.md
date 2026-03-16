## ADDED Requirements

### Requirement: Substring Extraction Operation
The string module SHALL provide a function to extract a substring from a source string given a starting index and length.

#### Scenario: Extract substring with valid parameters
- **WHEN** `fun_string_substring(source, start, length, output, output_size)` is called with valid parameters
- **THEN** the substring starting at index `start` with `length` characters is copied to `output`
- **AND** output is null-terminated

#### Scenario: Substring with start at beginning
- **WHEN** `fun_string_substring()` is called with `start = 0`
- **THEN** extraction begins from the first character of the source string

#### Scenario: Substring length exceeds remaining characters
- **WHEN** `fun_string_substring()` is called where `start + length > source_length`
- **THEN** function returns `ERROR_CODE_INDEX_OUT_OF_BOUNDS`

#### Scenario: Substring with NULL source
- **WHEN** `fun_string_substring()` is called with `source = NULL`
- **THEN** function returns `ERROR_CODE_NULL_POINTER`

#### Scenario: Substring with NULL output
- **WHEN** `fun_string_substring()` is called with `output = NULL`
- **THEN** function returns `ERROR_CODE_NULL_POINTER`

#### Scenario: Substring with insufficient output buffer
- **WHEN** `fun_string_substring()` is called where `output_size < length + 1`
- **THEN** function returns `ERROR_CODE_BUFFER_TOO_SMALL`

### Requirement: String Slice Operation
The string module SHALL provide a function to extract a slice of a string given start and end indices, supporting negative indices.

#### Scenario: Extract slice with positive indices
- **WHEN** `fun_string_slice(source, start, end, output, output_size)` is called with `0 <= start < end <= source_length`
- **THEN** characters from index `start` (inclusive) to `end` (exclusive) are copied to `output`
- **AND** output is null-terminated

#### Scenario: Slice with negative start index
- **WHEN** `fun_string_slice()` is called with negative `start` value
- **THEN** start index is calculated as `source_length + start` (offset from end)
- **AND** extraction proceeds normally with the calculated start

#### Scenario: Slice with negative end index
- **WHEN** `fun_string_slice()` is called with negative `end` value
- **THEN** end index is calculated as `source_length + end` (offset from end)
- **AND** extraction proceeds normally with the calculated end

#### Scenario: Slice with start >= end
- **WHEN** `fun_string_slice()` is called where `start >= end` (after resolving negatives)
- **THEN** function returns empty string in output (output[0] = '\0')

#### Scenario: Slice with out-of-bounds indices
- **WHEN** `fun_string_slice()` is called with `start` or `end` beyond valid range (after resolving negatives)
- **THEN** function returns `ERROR_CODE_INDEX_OUT_OF_BOUNDS`

#### Scenario: Slice with NULL parameters
- **WHEN** `fun_string_slice()` is called with `source = NULL` or `output = NULL`
- **THEN** function returns `ERROR_CODE_NULL_POINTER`

#### Scenario: Slice with insufficient output buffer
- **WHEN** `fun_string_slice()` is called where `output_size < (end - start) + 1`
- **THEN** function returns `ERROR_CODE_BUFFER_TOO_SMALL`
