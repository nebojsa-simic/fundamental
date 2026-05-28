## Context

The Fundamental Library has hand-rolled parsers for INI, TSV, CLI args, boolean/integer values, and network addresses. All follow the same pattern: in-place buffer modification with null-terminator insertion, character-by-character scanning, and zero mandatory heap allocation. The TSV module (`tsv.h`) is the closest design precedent with its stack-allocatable parser state and `init→next` streaming iteration. This design extends that pattern to JSON.

No existing module needs modification. The parser is pure computation — no platform-specific code.

## Goals / Non-Goals

**Goals:**
- Streaming tokenizer that modifies the input buffer in-place (inserts `\0` terminators for strings, numbers, keys)
- Stack-allocatable parser state — zero mandatory heap allocation
- Token carries pointer + length into the mutated buffer (O(1) value access, no extra scans)
- Depth tracking, parent context (`is_array`, `array_index`) baked into each token to eliminate caller-side state management
- Path-based positional init (`fun_json_init_at_path`) to start iteration at a nested key/array index
- Depth-aware iteration helpers (`fun_json_next_at`, `fun_json_skip_value`, `fun_json_find_key`) to make nested structure traversal concise
- Path-based single-shot query (`fun_json_query`) that scans only up to the matching token then stops
- Typed extractors (`_as_int`, `_as_double`, `_as_bool`, `_is_null`, `_copy_string`) operating on any token
- Convenience combinators (`fun_json_query_string`, `_int`, `_double`, `_bool`) for one-call config lookups
- Strict JSON grammar with sensible error reporting (specific error codes per failure mode)

**Non-Goals:**
- DOM/tree mode — callers build trees from tokens if needed, the module does not allocate one
- Serialization / pretty-printing
- Schema validation
- JSON5 extensions (comments, trailing commas, unquoted keys, single-quote strings) — defer to v2
- Async I/O — the parser operates on a memory buffer; file reading is the caller's responsibility

## Decisions

### 1. In-place buffer mutation over copy-out

**Rationale:** TSV and INI parsers both modify the buffer in-place, replacing delimiters with `\0`. This produces zero-copy C strings for keys and values that live as long as the input buffer. Every existing Fundamental parser follows this pattern.

**Alternative:** Copy each value into a caller-provided buffer. Rejected because it forces the caller to know the maximum value length in advance and doubles memory traffic for every token.

### 2. Two-layer API (Tokenizer + Query)

**Rationale:** Layer 1 (raw token stream) serves transformers and validators. Layer 2 (path query) serves config consumers who need single values by path. Build Layer 2 on Layer 1 internally — zero code duplication, zero extra allocation.

**Alternative:** Single API that's either too low-level for config consumers or too opinionated for transformers. Two layers give each use case the right abstraction.

### 3. Token carries parent context (`parent_is_array`, `array_index`)

**Rationale:** Without this the caller must maintain a parallel depth/is_array stack to know when tokens belong inside an array vs an object. The parser already tracks this internally — exposing it costs nothing.

**Alternative:** Caller tracks context manually. Rejected for poor dev ergonomics — the whole point of the array helper design is eliminating caller state.

### 4. `fun_json_init_at_path` positions cursor via prefix scan

**Rationale:** A config consumer wants to iterate `apps.http.servers.restic.routes[]` — without this, they'd iterate every token from the document root and manually filter by depth/key. This scans only the path prefix (~15 tokens for a 5-level path) then positions the cursor inside the target container. The caller uses `base_depth` as their natural loop exit condition.

**Alternative:** Parse the whole document into a token list, then index into it. Rejected — requires heap allocation for the token list.

### 5. `fun_json_find_key` + `fun_json_next_at` + `fun_json_skip_value` for nested iteration

**Rationale:** Raw Layer 1 requires explicit depth tracking and key matching in user code. With these three helpers, iterating nested arrays and objects becomes depth-guided (the caller only needs to know the relative depth of their target).

### 6. Typed extractors operate on any token

**Rationale:** `fun_json_token_as_int()` works on any token's `value`/`length` pair — no need for a separate "number result" type. This keeps the token type flat and extractors orthogonal.

### 7. Path syntax: dot for keys, integer index for arrays

**Rationale:** Familiar from JavaScript. `apps.http.servers.restic.routes.0.handle.1.handler` is instantly readable. No special characters, no quoting needed.

### 8. Error code block 270-278

**Rationale:** Thread pool ends at 252, config sits at 220. The 253-269 gap is available but starting at 270 gives room if other modules claim the lower gap. Seven codes cover the known failure modes.

### 9. No `arch/` code

**Rationale:** JSON parsing is pure character scanning and arithmetic on a buffer. No syscalls, no platform abstractions. Same as `src/tsv/tsv.c`.

### 10. Pointer + length in token (not just null-terminated pointer)

**Rationale:** The parser counts bytes as it scans a string/number literal. Exposing length avoids redundant `fun_string_length()` calls and enables `fun_memory_compare` for key matching without `\0` comparison.

## Risks / Trade-offs

- **[Repeated scanning for multiple queries]** → Each `fun_json_query` re-scans the path prefix. For 1-4KB config files this is sub-millisecond. Callers needing 100+ queries on large documents should use Layer 1 iteration with manual key tracking. The API surface makes this migration trivial (switch from `query_string` to `init_at_path` + loop).
- **[Buffer mutation is destructive]** → Caller must copy the input if they need the original. Documented clearly in header and skill. Same trade-off as TSV, INI parser, and every other Fundamental parser.
- **[Nesting depth limit]** → Hard cap at `FUN_JSON_MAX_DEPTH` (default 32). Overflow returns error code 273. Config documents rarely exceed 10-15 levels.
- **[Number overflow/loss]** → `fun_json_token_as_int` returns error if the number exceeds `INT64_MAX`. `fun_json_token_as_double` may lose precision for very large integers (>2^53). Documented behavior.

## Open Questions

<!-- Resolved during design phase — no open questions remain -->
