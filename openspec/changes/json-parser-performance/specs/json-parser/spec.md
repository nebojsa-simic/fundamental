## ADDED Requirements

### Requirement: JSON Parser Initialisation
The JSON module SHALL initialise a streaming parser over a caller-provided mutable buffer without allocating memory.

#### Scenario: Successful initialisation
- **WHEN** `fun_json_init(FunJsonParser *parser, char *data, uint64_t len)` is called with a non-null parser, non-null mutable buffer, and valid length
- **THEN** the parser is ready to yield tokens from the buffer
- **AND** no heap allocation occurs

#### Scenario: Null parser
- **WHEN** `fun_json_init` is called with a null parser pointer
- **THEN** an error result with code `ERROR_CODE_NULL_POINTER` is returned

#### Scenario: Null data
- **WHEN** `fun_json_init` is called with a null data pointer
- **THEN** an error result with code `ERROR_CODE_NULL_POINTER` is returned

#### Scenario: Empty input
- **WHEN** `fun_json_init` is called with zero length
- **THEN** the parser initialises successfully
- **AND** the first call to `fun_json_next` returns the end-of-input sentinel token

### Requirement: JSON Token Iteration
The JSON module SHALL yield one token at a time from the parsed buffer with type, value pointer, byte length, nesting depth, and parent context.

#### Scenario: Next token yields structured data
- **WHEN** `fun_json_next(FunJsonParser *parser)` is called after successful init
- **THEN** the result contains a `FunJsonToken` with `type`, `value`, `length`, `depth`, `parent_is_array`, and `array_index` fields populated
- **AND** STRING, KEY, and NUMBER tokens carry the value pointer and byte length into the mutated buffer

#### Scenario: Token sequence for a simple object
- **WHEN** parsing `{"key":"value"}`
- **THEN** tokens are yielded in order: OBJECT_START, KEY ("key"), STRING ("value"), OBJECT_END
- **AND** KEY and STRING depth is 1

#### Scenario: Token sequence for a nested array
- **WHEN** parsing `[[1,2],[3]]`
- **THEN** tokens include ARRAY_START(d=1), NUMBER("1",d=2), NUMBER("2",d=2), ARRAY_END(d=1), ARRAY_START(d=1), NUMBER("3",d=2), ARRAY_END(d=1)
- **AND** numbers inside the first inner array have `parent_is_array=true` and `array_index` set to 0 and 1 respectively

#### Scenario: End of input
- **WHEN** `fun_json_next` is called after all tokens have been consumed
- **THEN** the returned token has type `FUN_JSON_TOKEN_END`

### Requirement: JSON Primitive Value Parsing
The JSON module SHALL parse string, number, boolean, and null values and expose them as typed tokens.

#### Scenario: String value
- **WHEN** parsing `"hello world"`
- **THEN** a STRING token is yielded with `value="hello world"` and `length=11`
- **AND** the quotes are replaced with `\0` in the buffer

#### Scenario: Integer value
- **WHEN** parsing `42`
- **THEN** a NUMBER token is yielded with `value="42"` and `length=2`

#### Scenario: Negative number
- **WHEN** parsing `-17`
- **THEN** a NUMBER token is yielded with `value="-17"` and `length=3`

#### Scenario: Floating-point number
- **WHEN** parsing `3.14`
- **THEN** a NUMBER token is yielded with `value="3.14"` and `length=4`

#### Scenario: Boolean true
- **WHEN** parsing `true`
- **THEN** a BOOL token is yielded with no value (type alone is sufficient)

#### Scenario: Boolean false
- **WHEN** parsing `false`
- **THEN** a BOOL token is yielded

#### Scenario: Null
- **WHEN** parsing `null`
- **THEN** a NULL token is yielded

### Requirement: JSON String Escape Handling
The JSON module SHALL unescape standard JSON escape sequences within string literals in-place.

#### Scenario: Escaped quote
- **WHEN** parsing `"he\"llo"`
- **THEN** the token value is `he"llo` with length 6
- **AND** the escape sequence characters are compacted in-place

#### Scenario: Escaped backslash
- **WHEN** parsing `"a\\b"`
- **THEN** the token value is `a\b` with length 3

#### Scenario: Escaped newline
- **WHEN** parsing `"line\nbreak"`
- **THEN** the token value contains a literal newline character (byte 0x0A)

### Requirement: JSON Error Detection
The JSON module SHALL detect and report malformed JSON with specific error codes.

#### Scenario: Unterminated string
- **WHEN** `fun_json_next` encounters a string literal with no closing quote before end of buffer
- **THEN** an error result with code `ERROR_CODE_JSON_UNTERMINATED_STRING` (271) is returned

#### Scenario: Invalid number format
- **WHEN** `fun_json_next` encounters a malformed number (e.g., `01` or `1.` or `1e`)
- **THEN** an error result with code `ERROR_CODE_JSON_INVALID_NUMBER` (272) is returned

#### Scenario: Unexpected token
- **WHEN** `fun_json_next` encounters a character that does not start a valid JSON token
- **THEN** an error result with code `ERROR_CODE_JSON_UNEXPECTED_TOKEN` (276) is returned

#### Scenario: Missing colon after key
- **WHEN** `fun_json_next` encounters a key name not followed by `:`
- **THEN** an error result with code `ERROR_CODE_JSON_MISSING_COLON` (275) is returned

#### Scenario: Missing comma between elements
- **WHEN** `fun_json_next` encounters two values in an array or object without an intervening comma
- **THEN** an error result with code `ERROR_CODE_JSON_MISSING_COMMA` (277) is returned

#### Scenario: Nesting too deep
- **WHEN** `fun_json_next` encounters nesting exceeding `FUN_JSON_MAX_DEPTH`
- **THEN** an error result with code `ERROR_CODE_JSON_NESTING_TOO_DEEP` (273) is returned

#### Scenario: General parse error
- **WHEN** `fun_json_next` encounters any other structural violation of the JSON grammar
- **THEN** an error result with code `ERROR_CODE_JSON_PARSE_ERROR` (270) is returned

### Requirement: Path-Based Parser Initialisation
The JSON module SHALL allow initialising the parser at a specific path within the document, positioning the cursor inside the target container.

#### Scenario: Init at object key
- **WHEN** `fun_json_init_at_path(parser, data, len, "apps.http", &depth)` is called
- **THEN** the parser cursor is positioned at the value of key "http" inside "apps"
- **AND** `depth` is set to the depth of the "http" value
- **AND** the next call to `fun_json_next` yields the first token inside the "http" object

#### Scenario: Init at array index
- **WHEN** `fun_json_init_at_path(parser, data, len, "routes.0", &depth)` is called and path segment "0" targets an array
- **THEN** the parser cursor is positioned at the first element of the "routes" array
- **AND** `depth` is set to the depth of that array element

#### Scenario: Path segment not found
- **WHEN** `fun_json_init_at_path` is called with a key that does not exist in the document
- **THEN** an error result with code `ERROR_CODE_JSON_PATH_NOT_FOUND` (278) is returned

#### Scenario: Path segment indexes non-array
- **WHEN** `fun_json_init_at_path` is called with a numeric path segment targeting a value that is not an array
- **THEN** an error result with code `ERROR_CODE_JSON_PATH_NOT_FOUND` (278) is returned

### Requirement: Depth-Aware Iteration Helpers
The JSON module SHALL provide helpers that operate relative to a caller-specified depth, simplifying nested structure traversal.

#### Scenario: next_at skips below threshold
- **WHEN** `fun_json_next_at(parser, 3)` is called and the next token is at depth 4 or deeper
- **THEN** all tokens below depth 3 are skipped internally
- **AND** the returned token is the first one at or above depth 3

#### Scenario: next_at returns array end
- **WHEN** `fun_json_next_at(parser, depth)` is called inside an array and the next significant token is ARRAY_END at the given depth
- **THEN** the ARRAY_END token is returned

#### Scenario: skip_value consumes subtree
- **WHEN** `fun_json_skip_value(parser)` is called on an OBJECT_START token
- **THEN** all nested tokens up to and including the matching OBJECT_END are consumed
- **AND** the next call to `fun_json_next` returns the token after the skipped subtree

#### Scenario: find_key locates named member
- **WHEN** `fun_json_find_key(parser, depth, "handler", 7)` is called while positioned inside an object
- **THEN** all sibling members are scanned until a KEY token matching "handler" at the given depth is found
- **AND** the returned token is the value following that key

### Requirement: Single-Shot Path Query
The JSON module SHALL provide a single function to locate and return a token at a given dot-separated path, stopping scan at the match.

#### Scenario: Query finds string value
- **WHEN** `fun_json_query(data, len, "apps.http.servers.restic.routes.0.handle.1.handler")` is called on the caddy config
- **THEN** a STRING token with `value="restic"` and `length=6` is returned

#### Scenario: Query finds array
- **WHEN** `fun_json_query(data, len, "apps.http.servers.restic.routes")` is called
- **THEN** an ARRAY_START token at the correct depth is returned

#### Scenario: Query path not found
- **WHEN** `fun_json_query(data, len, "nonexistent.key")` is called
- **THEN** an error result with code `ERROR_CODE_JSON_PATH_NOT_FOUND` (278) is returned

#### Scenario: Query stops early
- **WHEN** `fun_json_query` finds a match before reaching end of document
- **THEN** scanning stops at the matched token
- **AND** tokens after the match are not parsed

### Requirement: Typed Value Extractors
The JSON module SHALL provide functions to extract typed values from any token without additional allocation.

#### Scenario: Extract string from token
- **WHEN** `fun_json_token_copy_string(token, out, out_size)` is called on a STRING or KEY token
- **THEN** the value is copied into `out` with a null terminator
- **AND** if `out_size` is too small, error `ERROR_CODE_BUFFER_TOO_SMALL` is returned

#### Scenario: Extract int from number token
- **WHEN** `fun_json_token_as_int(token)` is called on a NUMBER token with value "42"
- **THEN** the result is `int64_t` 42

#### Scenario: Extract double from number token
- **WHEN** `fun_json_token_as_double(token)` is called on a NUMBER token with value "3.14"
- **THEN** the result is `double` 3.14

#### Scenario: Extract bool from bool token
- **WHEN** `fun_json_token_as_bool(token)` is called on a BOOL token with value `true`
- **THEN** the result is `true`

#### Scenario: Type mismatch on extract
- **WHEN** `fun_json_token_as_int(token)` is called on a STRING token
- **THEN** an error result with code `ERROR_CODE_JSON_TYPE_MISMATCH` (279) is returned

### Requirement: Convenience Query-Extract Combinators
The JSON module SHALL provide single-call functions that combine path query with typed extraction.

#### Scenario: Query string in one call
- **WHEN** `fun_json_query_string(data, len, "handler", out, out_size)` is called
- **THEN** the string value at the given path is copied into `out`
- **AND** no intermediate token handling is required by the caller

#### Scenario: Query int in one call
- **WHEN** `fun_json_query_int(data, len, "port")` is called on `{"port":8080}`
- **THEN** the result is `int64_t` 8080

#### Scenario: Query convenience type mismatch
- **WHEN** `fun_json_query_string(data, len, "routes")` is called and the target is an ARRAY_START
- **THEN** an error result with code `ERROR_CODE_JSON_TYPE_MISMATCH` (279) is returned

### Requirement: In-Place Buffer Mutation
The JSON module SHALL modify the input buffer by inserting null terminators at value boundaries and compacting escape sequences, preserving the validity of value pointers for the buffer's lifetime.

#### Scenario: Value pointers valid after parsing
- **WHEN** iteration completes successfully
- **THEN** all `token.value` pointers point to null-terminated substrings within the original buffer
- **AND** these pointers remain valid as long as the original buffer is not freed or overwritten

#### Scenario: Buffer modified only within value regions
- **WHEN** parsing completes
- **THEN** structural characters (`{`, `}`, `[`, `]`, `:`, `,`) may be overwritten
- **AND** value content (strings, numbers) occupies contiguous regions terminated by `\0`

### Requirement: No Mandatory Heap Allocation
The JSON module SHALL require no calls to `fun_memory_allocate` for basic operation; `FunJsonParser` SHALL be stack-allocatable.

#### Scenario: Stack allocation
- **WHEN** a caller declares `FunJsonParser parser` as a local variable
- **THEN** no calls to `fun_memory_allocate` are required before calling `fun_json_init`

#### Scenario: No internal allocation during iteration
- **WHEN** `fun_json_next` is called repeatedly until end of input
- **THEN** no heap allocation occurs at any point during iteration
