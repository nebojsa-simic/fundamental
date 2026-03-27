## Context

The filesystem module serialises `Path` objects to C strings using an internal helper `path_to_string_internal` that writes into a 512-byte stack buffer. Every filesystem operation (create directory, remove directory, list directory, file-exists, file-size, walk root) calls this helper and then passes the resulting string to the platform arch layer. When the helper silently truncated an overlong path, all downstream operations operated on a different, shorter path with no indication of the failure.

Several additional defects were identified at the arch layer:
- `create_parent_directories` in `src/filesystem/directory.c` hardcodes the `'/'` separator when restoring slashed characters after `mkdir`, causing mixed-separator paths on Windows.
- `fun_filesystem_remove_directory` maps the platform `-1` return (directory not empty) to `ERROR_CODE_PERMISSION_DENIED` via a catch-all branch.
- `fun_platform_directory_is_empty` (both arch layers) returns `true` when it cannot open the directory, causing the caller to incorrectly conclude the directory is empty.
- `walk_build_child_path` failures in `walk.c` silently skip entries with paths exceeding `FUN_WALK_PATH_SIZE`.

## Goals / Non-Goals

**Goals:**
- `path_to_string_internal` returns `ERROR_CODE_PATH_TOO_LONG` when the path would exceed the buffer, so all callers propagate the error naturally
- `create_parent_directories` uses `fun_path_separator()` so the correct separator is used on each platform
- `fun_filesystem_remove_directory` maps platform `-1` to `ERROR_CODE_DIRECTORY_NOT_EMPTY`
- `fun_platform_directory_is_empty` returns `0` (false / error) when the directory cannot be opened, rather than `1` (true/empty)
- Walk entries whose constructed path exceeds `FUN_WALK_PATH_SIZE` are skipped and this is documented as defined behaviour

**Non-Goals:**
- Increasing the 512-byte path buffer or switching to heap allocation
- Resolving symlinks or adding canonical-path support
- Fixing the hardcoded Linux errno constants (separate change)
- Addressing MAX_PATH on Windows (requires `\\?\` prefix work; separate change)

## Decisions

### 1. Return error from `path_to_string_internal` on truncation

**Decision**: Add a length check before each component write. If writing the component would exceed `buffer_size - 1`, write the null terminator and return `fun_error_result(ERROR_CODE_PATH_TOO_LONG, ...)`.

**Alternatives considered**:
- *Grow the buffer dynamically*: rejected — the library has no heap allocation in src/. The arch layer could use heap, but the problem is in the common layer.
- *Pass a larger fixed buffer (e.g. 4096 bytes)*: rejected — this just moves the cliff; the truncation bug remains for any size.
- *Add an `out_len` parameter*: not needed — callers only need success/failure to decide whether to proceed.

All existing callers already test the `ErrorResult` and return early on error, so they will propagate `ERROR_CODE_PATH_TOO_LONG` automatically with no further changes required.

### 2. Use `fun_path_separator()` in `create_parent_directories`

**Decision**: Replace the literal `'/'` at `path.c:161` with a call to `fun_path_separator()`.

This is a one-line fix with no trade-offs. The separator is already available through the platform abstraction.

### 3. Map platform `-1` to `ERROR_CODE_DIRECTORY_NOT_EMPTY` in `fun_filesystem_remove_directory`

**Decision**: Add an explicit `else if (result == -1)` branch before the catch-all `else if (result < 0)` in `directory.c`.

The platform layer already documents `-1` as "not empty", `-2` as "not found", `-3` as "not a directory". The catch-all was inadvertently absorbing the not-empty case.

### 4. `fun_platform_directory_is_empty` on open failure returns `0`

**Decision**: Change the Linux and Windows implementations to return `0` when the `open`/`FindFirstFile` call fails. The caller (`fun_filesystem_remove_directory`) tests `if (!fun_platform_directory_is_empty(...))` before proceeding — returning `0` causes it to return `ERROR_CODE_DIRECTORY_NOT_EMPTY` rather than proceeding to `rmdir`.

**Trade-off**: The error is surfaced as `ERROR_CODE_DIRECTORY_NOT_EMPTY` rather than a more precise "permission denied" or "I/O error". This is acceptable because: (a) the alternative (true/empty) was worse, and (b) the real `rmdir` call that follows would catch the real error anyway.

### 5. Walk skips entries with paths > `FUN_WALK_PATH_SIZE` (defined behaviour)

**Decision**: Keep the `continue` but document it as defined behaviour in the spec. The walk API returns `bool` from `fun_filesystem_walk_next`; adding an error channel would be an API break. A separate change can introduce an error-entry mechanism if needed.

**Rationale**: The 512-byte limit was always an implicit constraint. Making it explicit and documented is a meaningful improvement even without a remediation path in this change.

## Risks / Trade-offs

- **`ERROR_CODE_PATH_TOO_LONG` may not exist yet** → Check `filesystem.h`; add it if absent. This is a backwards-compatible addition (new constant, no signature change).
- **`create_parent_directories` separator fix could change behaviour on Windows** → Low risk: previously the function created directories with `/` on Windows, which may have worked accidentally on NTFS. The fix produces `\\`-separated paths, which is more correct.
- **Inaccessible directory reported as not-empty** → If a caller relies on `fun_platform_directory_is_empty` returning true for unreadable directories, this change is a behaviour break. No known callers outside `fun_filesystem_remove_directory`.

## Open Questions

*(none)*
