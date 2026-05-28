## Context

The Fundamental Library has hand-rolled parsers for INI, TSV, CLI args, boolean/integer values, and network addresses. All follow the same pattern: in-place buffer modification with null-terminator insertion, character-by-character scanning, and zero mandatory heap allocation. The TSV module (`tsv.h`) is the closest design precedent with its stack-allocatable parser state (`FunTsvState`) and `init→next` streaming iteration using output parameters (`FunTsvRow *row`). The string module defines `String` (const char *) and `OutputString` (char *) typedefs used throughout the library. This design extends these patterns to JSON.

No existing module needs modification. The parser is pure computation — no platform-specific code.

## Goals / Non-Goals

**Goals:**
- Streaming tokenizer (Layer 1) that modifies the input buffer in-place (`char *data`, inserts `\0` terminators for strings, numbers, keys)
- Non-mutating path query (Layer 2) that scans without modifying (`String data`, idempotent — repeatable on same data)
- Stack-allocatable parser state (`FunJsonState`) — zero mandatory heap allocation
- Output parameters for token-producing functions (`FunJsonToken *token` + `ErrorResult` return) — matches `fun_tsv_next(state, &row)` pattern
- `ErrorResult` for all no-value-return functions — avoids mixing `voidResult`/`ErrorResult` in one module
- Token uses `String value` typedef (const char *) — matches library convention over raw pointers
- Token carries `uint64_t length` alongside `value` (O(1) access, no extra scans)
- Depth tracking, parent context (`parent_is_array`, `array_index`) baked into each token to eliminate caller-side state management
- Path-based positional init (`fun_json_init_at_path`) to start iteration at a nested key/array index
- Depth-aware iteration helpers (`fun_json_next_at`, `fun_json_skip_value`, `fun_json_find_key`) for concise nested traversal
- Path-based single-shot query (`fun_json_query`) that scans only up to the matching token then stops
- Typed extractors (`fun_json_token_as_int` → `int64Result`, `_as_double` → `doubleResult`, `_as_bool` → `boolResult`, `_is_null` → `boolResult`) operating on any token
- Convenience combinators (`fun_json_query_string`, `_int`, `_double`, `_bool`) for one-call config lookups
- Strict JSON grammar with specific error codes per failure mode

**Non-Goals:**
- DOM/tree mode — callers build trees from tokens if needed, the module does not allocate one
- Serialization / pretty-printing
- Schema validation
- JSON5 extensions (comments, trailing commas, unquoted keys, single-quote strings) — defer to v2
- Async I/O — the parser operates on a memory buffer; file reading is the caller's responsibility

## Decisions

### 1. In-place buffer mutation for tokenizer (Layer 1)

**Rationale:** TSV and INI parsers both modify the buffer in-place, replacing delimiters with `\0`. This produces zero-copy C strings for keys and values that live as long as the input buffer. Every existing Fundamental parser follows this pattern. Tokenizer functions take `char *data`.

**Alternative:** Copy each value into a caller-provided buffer. Rejected because it forces the caller to know the maximum value length in advance and doubles memory traffic for every token.

### 2. Non-mutating scan for query functions (Layer 2)

**Rationale:** If query functions mutated the buffer, they'd be single-use — the second query would see a partially-mutated buffer and fail. Query functions take `String data` (const char *) and perform a lightweight non-mutating scan: track positions without inserting `\0`. This makes queries idempotent — caller can call `fun_json_query_string` 10 times on the same config doc.

The internal query scanner is simpler than the full tokenizer (~50 lines): no escape unescaping, no `\0` insertion, just position tracking. The returned token's `value` points into the original (unmodified) data and `length` tells how many bytes are valid.

**Alternative:** Copy the buffer before each query. Rejected — unnecessary allocation when the query scan is simpler and faster without mutation.

### 3. Output parameters for token-producing functions

**Rationale:** TSV uses `boolResult fun_tsv_next(FunTsvState *state, FunTsvRow *row)` — the row is an OUTPUT PARAMETER. Following this precedent, all JSON functions that produce a `FunJsonToken` use an output parameter with `ErrorResult` return:

```c
ErrorResult fun_json_next(FunJsonState *state, FunJsonToken *token);
ErrorResult fun_json_find_key(FunJsonState *state, uint64_t depth,
                               String key, FunJsonToken *token);
ErrorResult fun_json_query(String data, uint64_t len, String path,
                            FunJsonToken *token);
```

`FunJsonToken` is a small descriptor struct (not an owner of its data) — output parameter is the correct idiom.

**Alternative:** Return `FunJsonTokenResult` by value via `DEFINE_RESULT_TYPE(FunJsonToken)`. Rejected — inconsistent with TSV parser pattern, and the token points into external storage (doesn't own its data), making it a poor value type.

### 4. Consistent `ErrorResult` for no-value returns

**Rationale:** TSV uses `ErrorResult` for `fun_tsv_init`. String module uses `CanReturnError(void)`/`voidResult` for copy operations. Both are valid in the codebase, but mixing them in one module is confusing. JSON follows TSV precedent: ALL functions that return only error status use `ErrorResult`, none use `voidResult`.

### 5. `String` and `OutputString` typedefs over raw pointers

**Rationale:** The library defines `typedef const char *String` and `typedef char *OutputString` in `string.h`. All public interfaces use these typedefs. JSON follows the same convention:
- `FunJsonToken.value` → `String` (read-only view into buffer)
- Path parameters → `String` (null-terminated path strings like `"apps.http.servers"`)
- Output buffers → `OutputString` (mutable where function writes)
- Key parameter in `fun_json_find_key` → `String` (always a string literal or null-terminated token value; function computes `fun_string_length` internally)
- Mutable input buffers for tokenizer → `char *` (not String/OutputString — it's input that gets mutated in-place, a distinct role from both)

**Alternative:** Raw `const char *` and `char *`. Rejected — inconsistent with library conventions. The typedefs carry semantic meaning: `String` = "I will read this", `OutputString` = "I will write here", `char *` = "I will mutate this buffer in-place".

### 6. Token carries parent context (`parent_is_array`, `array_index`)

**Rationale:** Without this the caller must maintain a parallel depth/is_array stack to know when tokens belong inside an array vs an object. The parser already tracks this internally — exposing it costs nothing.

**Alternative:** Caller tracks context manually. Rejected for poor dev ergonomics — the whole point of the array helper design is eliminating caller state.

### 7. `fun_json_init_at_path` positions cursor via prefix scan

**Rationale:** A config consumer wants to iterate `apps.http.servers.restic.routes[]` — without this, they'd iterate every token from the document root and manually filter by depth/key. This scans only the path prefix (~15 tokens for a 5-level path) then positions the cursor inside the target container. The caller uses `base_depth` as their natural loop exit condition.

**Alternative:** Parse the whole document into a token list, then index into it. Rejected — requires heap allocation for the token list.

### 8. `fun_json_find_key` + `fun_json_next_at` + `fun_json_skip_value` for nested iteration

**Rationale:** Raw Layer 1 requires explicit depth tracking and key matching in user code. With these three helpers, iterating nested arrays and objects becomes depth-guided (the caller only needs to know the relative depth of their target).

### 9. Typed extractors operate on any token

**Rationale:** `fun_json_token_as_int()` works on any token's `value`/`length` pair — no need for a separate "number result" type. This keeps the token type flat and extractors orthogonal. Return types follow the config module pattern: `int64Result`, `doubleResult`, `boolResult`.

### 10. Path syntax: dot for keys, integer index for arrays

**Rationale:** Familiar from JavaScript. `apps.http.servers.restic.routes.0.handle.1.handler` is instantly readable. No special characters, no quoting needed.

### 11. Error code block 270-279

**Rationale:** Thread pool ends at 252, config sits at 220. Nine codes cover: PARSE_ERROR(270), UNTERMINATED_STRING(271), INVALID_NUMBER(272), NESTING_TOO_DEEP(273), MISSING_COLON(275), MISSING_COMMA(277), UNEXPECTED_TOKEN(276), PATH_NOT_FOUND(278), TYPE_MISMATCH(279).

### 12. No `arch/` code

**Rationale:** JSON parsing is pure character scanning and arithmetic on a buffer. No syscalls, no platform abstractions. Same as `src/tsv/tsv.c`.

### 13. `FunJsonState` type name (not `FunJsonParser`)

**Rationale:** TSV uses `FunTsvState` — parser state types use `*State` suffix. `FunJsonState` is consistent with the closest precedent.

## Risks / Trade-offs

- **[Repeated scanning for multiple queries]** → Each `fun_json_query` re-scans the path prefix. For 1-4KB config files this is sub-millisecond. Callers needing 100+ queries on large documents should use Layer 1 iteration with manual key tracking. The API surface makes this migration trivial (switch from `query_string` to `init_at_path` + loop).
- **[Buffer mutation is destructive (tokenizer only)]** → Caller must copy the input if they need the original. Documented clearly in header and skill. Query functions do NOT mutate — `String data` signals this at the type level.
- **[Query tokens are not null-terminated]** → Query scan does not insert `\0`. Caller must use `token.length` or copy via `fun_json_token_copy_string`. Acceptable: token always carries length.
- **[Nesting depth limit]** → Hard cap at `FUN_JSON_MAX_DEPTH` (default 32). Overflow returns error code 273. Config documents rarely exceed 10-15 levels.
- **[Number overflow/loss]** → `fun_json_token_as_int` returns error if the number exceeds `INT64_MAX`. `fun_json_token_as_double` may lose precision for very large integers (>2^53). Documented behavior.

## Open Questions

<!-- Resolved during design phase — no open questions remain -->
