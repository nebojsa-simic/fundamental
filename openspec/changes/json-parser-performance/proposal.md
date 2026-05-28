## Why

The Fundamental Library lacks any JSON parsing capability, blocking config formats, log analysis, and HTTP API consumption. This gap has been deferred in two archived designs. Adding a zero-allocation streaming JSON parser now unblocks these use cases while adhering to the library's performance-first, caller-allocated-memory philosophy.

## What Changes

- New module `json/` providing a streaming JSON tokenizer (Layer 1) and non-mutating path-based query helpers (Layer 2)
- Tokenizer mutates input buffer in-place (inserts `\0` at value boundaries) — zero-copy string extraction
- Query functions perform a non-mutating scan — idempotent, caller can query the same data multiple times
- 19 public functions covering tokenization, iteration, query, typed scalar extraction, and typed array extraction
- Go-style return values: token-producing functions use output parameters (`FunJsonToken *token`) + return `ErrorResult`; typed extractors return `int64Result`/`doubleResult`/`boolResult`/`uint64Result`
- New error code block: 270-279 covering parse errors, type mismatches, path-not-found, nesting overflow, and malformed input
- Header at `include/fundamental/json/json.h`, sources at `src/json/`, tests at `tests/json/`
- No `arch/` code required — pure computation, no OS calls
- Module depends only on `string.h` and `error.h`

## Capabilities

### New Capabilities

- `json-parser`: Zero-allocation streaming JSON tokenizer with path navigation, depth-aware iteration helpers, key search, subtree skip, typed value extraction, and typed array extraction. Tokenizer mutates input buffer in-place (`char *data`); query functions use a non-mutating scan (`String data`) for idempotent repeated lookups. Array convenience functions (`fun_json_query_int_array`, `fun_json_query_double_array`) copy JSON arrays into caller buffers in a single call. Supports both raw token iteration (Layer 1) for transformers and path-based single-shot queries (Layer 2) for config consumers.

### Modified Capabilities

<!-- None — no existing spec behavior changes -->

## Impact

- New files: `include/fundamental/json/json.h`, `src/json/` (tokenizer.c, number.c, string.c), `tests/json/` (test_json.c + build scripts)
- New error codes 270-279 in `include/fundamental/error/error.h` plus corresponding `ERROR_RESULT_JSON_*` static definitions
- Dependencies: `string`, `error` (no new dependencies introduced)
- No breaking changes to existing modules or APIs
