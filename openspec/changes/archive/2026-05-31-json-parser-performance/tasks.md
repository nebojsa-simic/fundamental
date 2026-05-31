## 1. Error Codes and Type Definitions

- [x] 1.1 Add error codes 270-279 to `include/fundamental/error/error.h` (PARSE_ERROR, UNTERMINATED_STRING, INVALID_NUMBER, NESTING_TOO_DEEP, MISSING_COLON, MISSING_COMMA, UNEXPECTED_TOKEN, PATH_NOT_FOUND, TYPE_MISMATCH)
- [x] 1.2 Add corresponding `ERROR_RESULT_JSON_*` static definitions in `include/fundamental/error/error.h`

## 2. Module Header

- [x] 2.1 Create `include/fundamental/json/json.h` with header guard `LIBRARY_JSON_H`
- [x] 2.2 Define `FunJsonTokenType` enum (OBJECT_START, OBJECT_END, ARRAY_START, ARRAY_END, STRING, NUMBER, BOOL, NULL, KEY, TOKEN_END)
- [x] 2.3 Define `FunJsonToken` struct: type, String value, uint64_t length, uint64_t depth, bool parent_is_array, uint64_t array_index
- [x] 2.4 Define `FunJsonState` stack-allocatable struct (data buffer pointer, pos, len, depth, cursor state)
- [x] 2.5 Define `FUN_JSON_MAX_DEPTH` constant (32)
- [x] 2.6 Declare all 17 public function signatures with `CanReturnError` / `DEFINE_RESULT_TYPE` patterns
- [x] 2.7 Add `#include` dependencies: `error.h`, `string.h`, `<stddef.h>`, `<stdbool.h>`, `<stdint.h>`

## 3. Core Tokenizer

- [x] 3.1 Create `src/json/tokenizer.c` with `fun_json_init` — initialise parser state from buffer + length
- [x] 3.2 Implement `fun_json_init_at_path` — scan path prefix, position cursor at target depth
- [x] 3.3 Implement character-scanning helpers: skip whitespace, peek/advance, detect digit/alpha
- [x] 3.4 Implement `fun_json_next` main token dispatch loop: switch on current character → delegate to type-specific parser

## 4. String Parser

- [x] 4.1 Create `src/json/string.c` with internal `json_parse_string()` — scan quoted string, unescape in-place, return pointer+length
- [x] 4.2 Handle escape sequences: `\"`, `\\`, `\/`, `\b`, `\f`, `\n`, `\r`, `\t`, `\uXXXX`
- [x] 4.3 Detect unterminated string (no closing quote before buffer end) → error 271

## 5. Number Parser

- [x] 5.1 Create `src/json/number.c` with internal `json_parse_number()` — scan integer, fraction, exponent parts, return pointer+length
- [x] 5.2 Validate number grammar: no leading zeros (except `0`), fraction requires digit after `.`, exponent requires digit after `e`/`E`
- [x] 5.3 Detect invalid number format → error 272

## 6. Structural Token Parsing

- [x] 6.1 Parse `{` → OBJECT_START, increment depth, check nesting limit → error 273 on overflow
- [x] 6.2 Parse `}` → OBJECT_END, decrement depth
- [x] 6.3 Parse `[` → ARRAY_START, increment depth, reset array_index, check nesting limit → error 273
- [x] 6.4 Parse `]` → ARRAY_END, decrement depth
- [x] 6.5 Parse `:` — required between KEY and value → error 275 if absent
- [x] 6.6 Parse `,` — required between elements → error 277 if absent (detected on next token)

## 7. Literal Parsing

- [x] 7.1 Implement `json_parse_true()` — match `t`, `r`, `u`, `e` → BOOL token
- [x] 7.2 Implement `json_parse_false()` — match `f`, `a`, `l`, `s`, `e` → BOOL token
- [x] 7.3 Implement `json_parse_null()` — match `n`, `u`, `l`, `l` → NULL token
- [x] 7.4 Detect partial literal match (`fal`, `tru`, `nul`) → error 276 (UNEXPECTED_TOKEN)

## 8. Token Context Tracking

- [x] 8.1 Track `parent_is_array` per depth level — set true when inside `[...]`, false inside `{...}`
- [x] 8.2 Track `array_index` per depth level — increment on each comma-separated element inside `[...]`
- [x] 8.3 Populate `token.depth`, `token.parent_is_array`, `token.array_index` on every yielded token

## 9. Depth-Aware Iteration Helpers

- [x] 9.1 Implement `fun_json_next_at` — yield next token at or above given depth, skip deeper subtrees
- [x] 9.2 Implement `fun_json_skip_value` — consume all tokens until depth returns to current level
- [x] 9.3 Implement `fun_json_find_key` — scan keys at current object level, return value of first match

## 10. Path Query

- [x] 10.1 Implement `fun_json_query` — parse dot-separated path, scan tokens until match, return token
- [x] 10.2 Handle numeric path segments as array indices (`routes.0` → first element)
- [x] 10.3 Return error 278 (PATH_NOT_FOUND) if key missing or index out of bounds

## 11. Typed Extractors

- [x] 11.1 Implement `fun_json_token_copy_string` — copy `value[0..length]` + null into caller buffer
- [x] 11.2 Implement `fun_json_token_as_int` — parse NUMBER value as int64, return error on overflow or non-numeric token
- [x] 11.3 Implement `fun_json_token_as_double` — parse NUMBER value as double
- [x] 11.4 Implement `fun_json_token_as_bool` — return bool equivalent, error on non-BOOL token
- [x] 11.5 Implement `fun_json_token_is_null` — return true if token type is NULL

## 12. Array Extractors

- [x] 12.1 Implement `fun_json_query_int_array` — walk JSON array via non-mutating scan, copy int64_t elements into caller buffer, return count
- [x] 12.2 Implement `fun_json_query_double_array` — walk JSON array via non-mutating scan, copy double elements into caller buffer, return count
- [x] 12.3 Implement `fun_json_query_string_array` — walk JSON array via non-mutating scan, copy string values sequentially into flat caller buffer (\0-separated), return count
- [x] 12.4 Handle errors: path-not-found → 278, target-not-array → 279, non-string element → 279
- [x] 12.5 Cap output at `buffer_size` — stop on overflow, never truncate mid-string; int/double cap at `max_count`

## 13. Convenience Combinators

- [x] 13.1 Implement `fun_json_query_string` — query path + copy string into caller buffer
- [x] 13.2 Implement `fun_json_query_int` — query path + parse as int64
- [x] 13.3 Implement `fun_json_query_double` — query path + parse as double
- [x] 13.4 Implement `fun_json_query_bool` — query path + parse as bool

## 14. Test Suite

- [x] 14.1 Create `tests/json/test_json.c` with test infrastructure (GREEN_CHECK, RED_CROSS, assert helpers)
- [x] 14.2 Test `fun_json_init` — null args, empty input, valid init
- [x] 14.3 Test `fun_json_next` — complete token sequence for `{"key":"value"}`, depth and parent context
- [x] 14.4 Test all token types — strings, ints, floats, booleans, null, nested objects, nested arrays
- [x] 14.5 Test string escaping — `\"`, `\\`, `\n`, `\t`, `\uXXXX`
- [x] 14.6 Test error cases — unterminated string, invalid number, missing colon, missing comma, nesting overflow
- [x] 14.7 Test `fun_json_init_at_path` — valid path, non-existent key, array index, wrong type
- [x] 14.8 Test `fun_json_next_at` — skipping subtrees, array traversal
- [x] 14.9 Test `fun_json_skip_value` — object subtree, array subtree, nested skip
- [x] 14.10 Test `fun_json_find_key` — key found, key absent, sibling keys ignored
- [x] 14.11 Test `fun_json_query` — valid path, non-existent path, type mismatch
- [x] 14.12 Test typed extractors — int parse, double parse, bool parse, null check, string copy, type mismatch
- [x] 14.13 Test convenience combinators — query_string, query_int, query_double, query_bool
- [x] 14.14 Test array extractors — int array, double array, string array, non-array target, wrong element type, buffered output, empty array, truncation/overflow
- [x] 14.15 Test in-place buffer mutation — verify tokenizer inserts null terminators into original buffer
- [x] 14.16 Test non-mutating query — verify data buffer is unmodified after `fun_json_query` and repeated queries on same data succeed
- [x] 14.17 Test with the full Caddy JSON config from the proposal — iterate routes via `fun_json_init_at_path`, query individual values via `fun_json_query_string`

## 15. Build Scripts

- [x] 15.1 Create `tests/json/build-linux-amd64.sh` with explicit source listing
- [x] 15.2 Create `tests/json/build-windows-amd64.bat` with explicit source listing
- [x] 15.3 Verify clean compile on Linux with `-Wall -Wextra -g -O0`
- [x] 15.4 Verify clean compile on Windows with `-Wall -Wextra -g -O0`

## 16. Validation

- [x] 16.1 Run `openspec validate json-parser-performance` to verify all artifacts
- [x] 16.2 Run full test suite on Linux: `./tests/json/build-linux-amd64.sh && ./tests/json/test`
- [x] 16.3 Run full test suite on Windows: `tests\json\build-windows-amd64.bat && tests\json\test.exe`
- [x] 16.4 Format all new files with `clang-format -i -style=file`
