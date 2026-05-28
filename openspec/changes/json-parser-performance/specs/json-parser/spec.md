## ADDED Requirements

### Requirement: JSON Parser State
The JSON module SHALL provide a stack-allocatable parser state type (`FunJsonState`) requiring no mandatory heap allocation.

#### Scenario: Stack allocation
- **WHEN** a caller declares `FunJsonState state` as a local variable
- **THEN** no calls to `fun_memory_allocate` are required before calling `fun_json_init`
- **AND** `fun_json_init` uses only the stack-allocated struct, not the heap

#### Scenario: No internal allocation during iteration
- **WHEN** `fun_json_next` is called repeatedly until end of input
- **THEN** no heap allocation occurs at any point during iteration

### Requirement: JSON Tokenizer Initialisation
The JSON module SHALL initialise a streaming tokenizer over a caller-provided mutable buffer.

#### Scenario: Successful initialisation
- **WHEN** `ErrorResult fun_json_init(FunJsonState *state, char *data, uint64_t len)` is called with a non-null state, non-null mutable buffer, and valid length
- **THEN** the tokenizer is ready to yield tokens from the buffer
- **AND** the buffer WILL BE modified in-place during tokenization (`\0` inserted at value boundaries)
- **AND** no heap allocation occurs

#### Scenario: Null state
- **WHEN** `fun_json_init` is called with a null state pointer
- **THEN** an error result with code `ERROR_CODE_NULL_POINTER` is returned

#### Scenario: Null data
- **WHEN** `fun_json_init` is called with a null data pointer
- **THEN** an error result with code `ERROR_CODE_NULL_POINTER` is returned

#### Scenario: Empty input
- **WHEN** `fun_json_init` is called with zero length
- **THEN** the tokenizer initialises successfully
- **AND** the first call to `fun_json_next` yields a token with type `FUN_JSON_TOKEN_END`

### Requirement: JSON Token Iteration
The JSON module SHALL yield one token at a time via an output parameter with `ErrorResult` return.

#### Scenario: Function signature matches TSV pattern
- **WHEN** a caller invokes `ErrorResult fun_json_next(FunJsonState *state, FunJsonToken *token)`
- **THEN** `token` is populated with type, value (String), length, depth, parent_is_array, and array_index
- **AND** the return value is `ErrorResult` — check `fun_error_is_error(err)` for parse errors
- **AND** `token.value` uses the `String` typedef (const char *) consistent with library convention

#### Scenario: Token sequence for a simple object
- **WHEN** parsing `{"key":"value"}`
- **THEN** tokens are yielded in order: OBJECT_START, KEY ("key"), STRING ("value"), OBJECT_END
- **AND** KEY and STRING depth is 1
- **AND** value pointers point into the mutated input buffer

#### Scenario: Token sequence for a nested array
- **WHEN** parsing `[[1,2],[3]]`
- **THEN** tokens include ARRAY_START(d=1), NUMBER("1",d=2), NUMBER("2",d=2), ARRAY_END(d=1), ARRAY_START(d=1), NUMBER("3",d=2), ARRAY_END(d=1)
- **AND** numbers inside the first inner array have `parent_is_array=true` and `array_index` set to 0 and 1 respectively
- **AND** the number inside the second inner array has `array_index=0`

#### Scenario: End of input
- **WHEN** `fun_json_next` is called after all tokens have been consumed
- **THEN** the returned token has type `FUN_JSON_TOKEN_END`
- **AND** the `ErrorResult` indicates no error

### Requirement: JSON Primitive Value Parsing
The JSON module SHALL parse string, number, boolean, and null values and expose them as typed tokens.

#### Scenario: String value
- **WHEN** parsing `"hello world"`
- **THEN** a STRING token is yielded with `value="hello world"` and `length=11`
- **AND** the surrounding quotes are replaced with `\0` in the buffer (tokenizer mode)

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
- **THEN** a BOOL token is yielded

#### Scenario: Boolean false
- **WHEN** parsing `false`
- **THEN** a BOOL token is yielded

#### Scenario: Null
- **WHEN** parsing `null`
- **THEN** a NULL token is yielded

### Requirement: JSON String Escape Handling
The JSON module SHALL unescape standard JSON escape sequences within string literals in-place during tokenization.

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

#### Scenario: Escaped tab
- **WHEN** parsing `"col1\tcol2"`
- **THEN** the token value contains a literal tab character (byte 0x09)

#### Scenario: Unicode escape
- **WHEN** parsing `"\u0041"`
- **THEN** the token value is `A` with length 1

### Requirement: JSON Error Detection
The JSON module SHALL detect and report malformed JSON with specific error codes on `fun_json_next`.

#### Scenario: Unterminated string
- **WHEN** `fun_json_next` encounters a string literal with no closing quote before end of buffer
- **THEN** an `ErrorResult` with code `ERROR_CODE_JSON_UNTERMINATED_STRING` (271) is returned

#### Scenario: Invalid number format
- **WHEN** `fun_json_next` encounters a malformed number (e.g., `01` or `1.` or `1e`)
- **THEN** an `ErrorResult` with code `ERROR_CODE_JSON_INVALID_NUMBER` (272) is returned

#### Scenario: Unexpected token
- **WHEN** `fun_json_next` encounters a character that does not start a valid JSON token
- **THEN** an `ErrorResult` with code `ERROR_CODE_JSON_UNEXPECTED_TOKEN` (276) is returned

#### Scenario: Missing colon after key
- **WHEN** `fun_json_next` encounters a key name not followed by `:`
- **THEN** an `ErrorResult` with code `ERROR_CODE_JSON_MISSING_COLON` (275) is returned

#### Scenario: Missing comma between elements
- **WHEN** `fun_json_next` encounters two values in an array or object without an intervening comma
- **THEN** an `ErrorResult` with code `ERROR_CODE_JSON_MISSING_COMMA` (277) is returned

#### Scenario: Nesting too deep
- **WHEN** `fun_json_next` encounters nesting exceeding `FUN_JSON_MAX_DEPTH`
- **THEN** an `ErrorResult` with code `ERROR_CODE_JSON_NESTING_TOO_DEEP` (273) is returned

#### Scenario: General parse error
- **WHEN** `fun_json_next` encounters any other structural violation of the JSON grammar
- **THEN** an `ErrorResult` with code `ERROR_CODE_JSON_PARSE_ERROR` (270) is returned

### Requirement: Path-Based Tokenizer Initialisation
The JSON module SHALL allow initialising the tokenizer at a specific path within the document via `fun_json_init_at_path`, positioning the cursor inside the target container.

#### Scenario: Function signature
- **WHEN** a caller invokes `ErrorResult fun_json_init_at_path(FunJsonState *state, char *data, uint64_t len, String path, uint64_t *base_depth)`
- **THEN** `state` is initialised with cursor positioned at the path target
- **AND** `*base_depth` is set to the depth of the target container
- **AND** `path` uses the `String` typedef (null-terminated path like `"apps.http.servers"`)

#### Scenario: Init at object key
- **WHEN** `fun_json_init_at_path` is called with path `"apps.http"`
- **THEN** the cursor is positioned at the value of key "http" inside "apps"
- **AND** `base_depth` is set to the depth of the "http" value
- **AND** the next call to `fun_json_next` yields the first token inside the "http" object

#### Scenario: Init at array index
- **WHEN** `fun_json_init_at_path` is called with path `"routes.0"` and segment "0" targets an array element
- **THEN** the cursor is positioned at the first element of the "routes" array
- **AND** `base_depth` is set to the depth of that array element

#### Scenario: Path segment not found
- **WHEN** `fun_json_init_at_path` is called with a key that does not exist in the document
- **THEN** an error result with code `ERROR_CODE_JSON_PATH_NOT_FOUND` (278) is returned

#### Scenario: Path segment indexes non-array
- **WHEN** `fun_json_init_at_path` is called with a numeric path segment targeting a value that is not an array
- **THEN** an error result with code `ERROR_CODE_JSON_PATH_NOT_FOUND` (278) is returned

### Requirement: Depth-Aware Iteration Helpers
The JSON module SHALL provide helpers that operate relative to a caller-specified depth, all returning `ErrorResult`.

#### Scenario: next_at function signature
- **WHEN** a caller invokes `ErrorResult fun_json_next_at(FunJsonState *state, uint64_t depth, FunJsonToken *token)`
- **THEN** tokens below the given depth are skipped internally
- **AND** the returned token via `*token` is the first one at or above the given depth

#### Scenario: next_at returns array end
- **WHEN** `fun_json_next_at(state, depth, &token)` is called inside an array and the next significant token is ARRAY_END at the given depth
- **THEN** the ARRAY_END token is returned via `token.type`

#### Scenario: skip_value function signature
- **WHEN** a caller invokes `ErrorResult fun_json_skip_value(FunJsonState *state)` on an OBJECT_START or ARRAY_START token
- **THEN** all nested tokens up to and including the matching end token are consumed
- **AND** the next call to `fun_json_next` returns the token after the skipped subtree

#### Scenario: find_key function signature uses String typedef
- **WHEN** a caller invokes `ErrorResult fun_json_find_key(FunJsonState *state, uint64_t depth, String key, FunJsonToken *token)`
- **THEN** all sibling members are scanned until a KEY token matching `key` at the given depth is found
- **AND** `key` uses the `String` typedef — a null-terminated string literal or key value
- **AND** the function computes `fun_string_length(key)` internally for comparison
- **AND** the returned token via `*token` is the value following that key

### Requirement: Non-Mutating Path Query
The JSON module SHALL provide `fun_json_query` that scans the document without modifying the input buffer, returning a token via output parameter.

#### Scenario: Query function signature uses String typedefs
- **WHEN** a caller invokes `ErrorResult fun_json_query(String data, uint64_t len, String path, FunJsonToken *token)`
- **THEN** `data` uses the `String` typedef and is NOT modified (const)
- **AND** `path` uses the `String` typedef
- **AND** `*token` is populated with type, value (String), and length into the original data
- **AND** `token.value` is NOT null-terminated (caller must use `token.length` or `fun_json_token_copy_string`)

#### Scenario: Query finds string value
- **WHEN** `fun_json_query(data, len, "apps.http.servers.restic.routes.0.handle.1.handler", &token)` is called on the caddy config
- **THEN** a STRING token with `value="restic"` and `length=6` is returned

#### Scenario: Query finds array
- **WHEN** `fun_json_query(data, len, "apps.http.servers.restic.routes", &token)` is called
- **THEN** an ARRAY_START token at the correct depth is returned

#### Scenario: Query path not found
- **WHEN** `fun_json_query(data, len, "nonexistent.key", &token)` is called
- **THEN** an error result with code `ERROR_CODE_JSON_PATH_NOT_FOUND` (278) is returned

#### Scenario: Query stops early
- **WHEN** `fun_json_query` finds a match before reaching end of document
- **THEN** scanning stops at the matched token
- **AND** tokens after the match are not parsed

#### Scenario: Multiple queries are idempotent
- **WHEN** `fun_json_query` is called twice on the same `data` with different paths
- **THEN** both calls succeed
- **AND** the buffer is unmodified after both calls
- **AND** returned value pointers remain valid

### Requirement: Typed Value Extractors
The JSON module SHALL provide functions to extract typed values from any token without additional allocation. Extractor return types follow the config module pattern.

#### Scenario: extractor signatures
- **WHEN** calling `int64Result fun_json_token_as_int(FunJsonToken *token)` on NUMBER "42"
- **THEN** result is `int64_t` 42
- **AND** calling with null token returns `ERROR_CODE_NULL_POINTER`
- **AND** calling with STRING token returns `ERROR_CODE_JSON_TYPE_MISMATCH` (279)

#### Scenario: Extract double from number token
- **WHEN** `doubleResult fun_json_token_as_double(FunJsonToken *token)` is called on NUMBER "3.14"
- **THEN** the result is `double` 3.14

#### Scenario: Extract bool from bool token
- **WHEN** `boolResult fun_json_token_as_bool(FunJsonToken *token)` is called on BOOL true
- **THEN** the result is `true`

#### Scenario: Check null token
- **WHEN** `boolResult fun_json_token_is_null(FunJsonToken *token)` is called on NULL token
- **THEN** the result is `true`
- **AND** calling on STRING token returns `false` (not an error)

#### Scenario: Copy string from token uses OutputString
- **WHEN** `ErrorResult fun_json_token_copy_string(FunJsonToken *token, OutputString out, uint64_t out_size)` is called on STRING or KEY token
- **THEN** `out` uses the `OutputString` typedef and receives the null-terminated value
- **AND** if `out_size` is too small, `ERROR_CODE_BUFFER_TOO_SMALL` is returned
- **AND** works for both query tokens (non-null-terminated source) and tokenizer tokens (null-terminated source)

### Requirement: Convenience Query-Extract Combinators
The JSON module SHALL provide single-call functions that combine non-mutating query with typed extraction.

#### Scenario: Convenience signatures use String and OutputString
- **WHEN** calling `ErrorResult fun_json_query_string(String data, uint64_t len, String path, OutputString out, uint64_t out_size)`
- **THEN** `data` and `path` use `String` typedef, `out` uses `OutputString` typedef
- **AND** data is NOT modified
- **AND** `out` receives the null-terminated value at path
- **AND** the call is idempotent (repeatable on same data)

#### Scenario: Query int in one call
- **WHEN** `int64Result fun_json_query_int(String data, uint64_t len, String path)` is called on `{"port":8080}`
- **THEN** the result is `int64_t` 8080

#### Scenario: Query double in one call
- **WHEN** `doubleResult fun_json_query_double(String data, uint64_t len, String path)` is called on `{"pi":3.14}`
- **THEN** the result is `double` 3.14

#### Scenario: Query bool in one call
- **WHEN** `boolResult fun_json_query_bool(String data, uint64_t len, String path)` is called on `{"enabled":true}`
- **THEN** the result is `true`

#### Scenario: Query convenience type mismatch
- **WHEN** `fun_json_query_string(data, len, "routes", out, size)` is called and the target is an ARRAY_START
- **THEN** an error result with code `ERROR_CODE_JSON_TYPE_MISMATCH` (279) is returned

### Requirement: In-Place Buffer Mutation (Tokenizer Only)
The JSON module SHALL modify the input buffer during tokenization, and query functions SHALL NOT.

#### Scenario: Value pointers valid after tokenization
- **WHEN** tokenizer iteration completes successfully
- **THEN** all `token.value` pointers (String) point to null-terminated substrings within the original buffer
- **AND** these pointers remain valid as long as the original buffer is not freed or overwritten

#### Scenario: Query does not mutate
- **WHEN** `fun_json_query` or `fun_json_query_string` is called on `String data`
- **THEN** the data buffer is NOT modified (const-qualified at the type level)
- **AND** repeated calls on the same data are safe and produce correct results
