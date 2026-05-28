## Why

The Fundamental Library lacks any JSON parsing capability, blocking config formats, log analysis, and HTTP API consumption. This gap has been deferred in two archived designs. Adding a zero-allocation, in-place JSON parser now unblocks these use cases while adhering to the library's performance-first, caller-allocated-memory philosophy.

## What Changes

- New module `json/` providing a streaming JSON tokenizer (Layer 1) and path-based query helpers (Layer 2)
- 17 public functions covering tokenization, iteration, query, and typed value extraction
- New error code block: 270-278 covering parse errors, type mismatches, path-not-found, nesting overflow, and malformed input
- Header at `include/fundamental/json/json.h`, sources at `src/json/`, tests at `tests/json/`
- No `arch/` code required — pure computation, no OS calls
- Module depends only on `memory.h`, `string.h`, and `error.h`

## Capabilities

### New Capabilities

- `json-parser`: Zero-allocation, in-place streaming JSON5 tokenizer with path navigation, depth-aware iteration helpers, key search, subtree skip, and typed value extraction. Supports both raw token iteration (Layer 1) for transformers and path-based single-shot queries (Layer 2) for config consumers.

### Modified Capabilities

<!-- None — no existing spec behavior changes -->

## Impact

- New files: `include/fundamental/json/json.h`, `src/json/` (tokenizer.c, number.c, string.c), `tests/json/` (test_json.c + build scripts)
- New error codes 270-278 in `include/fundamental/error/error.h` plus corresponding `ERROR_RESULT_*` static definitions
- Dependencies: `string`, `memory`, `error` (no new dependencies introduced)
- No breaking changes to existing modules or APIs
