## Context

`fun_filesystem_list_directory` currently calls `fun_platform_directory_list` which iterates over OS directory entries and writes plain `name\n` strings into a caller-provided buffer, discarding the entry type (`d_type` on Linux, `dwFileAttributes` on Windows) that the OS provides for free. Callers (fundamental-cli `generator.c`, `discovery.c`) then re-implement recursive traversal with ad-hoc newline parsing, extra `fun_directory_exists` syscalls per entry, and depth-limited recursion. This duplication is ~200 lines across two files.

The library already follows a consistent pattern: caller-allocated buffers, opaque state structs, `fun_` prefixed streaming APIs, no stdlib in `src/` or `include/`.

## Goals / Non-Goals

**Goals:**
- TSV module: in-place streaming parser, stack-allocatable state, no allocation
- Walk API: one-entry-at-a-time streaming over a directory tree, fixed 40KB work memory, no truncation regardless of directory size
- Updated `fun_filesystem_list_directory` output includes type prefix, making the bulk listing API consistent with the walk's per-entry type info
- Breaking change to listing format is acceptable; fundamental-cli will be updated

**Non-Goals:**
- Recursive delete or copy utilities
- File watching / inotify
- Walk API on symlinks (treated as regular files)
- Sorting or ordering guarantees on yielded entries

## Decisions

### D1: Per-frame OS handle vs. shared listing buffer

**Decision**: Each walk depth level holds an open OS directory handle (`fun_platform_dir_open`). `fun_platform_dir_read_entry` reads one entry at a time directly from the OS.

**Rejected alternative**: Read the full directory listing into a shared buffer (8–16KB) and re-read the parent on ascent. This was the initial design but has two problems: (1) fixed buffer means large directories silently truncate, even with a configurable size; (2) re-reads on ascent add complexity and O(N×depth) extra syscalls.

**Rationale**: The OS already streams entries one at a time (`getdents64`, `FindNextFileW`). Holding an open handle per depth level costs at most `FUN_WALK_MAX_DEPTH` (16) file descriptors simultaneously — acceptable for a CLI tool. Memory is fixed and predictable.

### D2: FunDirHandle as fixed-size opaque blob

**Decision**: `unsigned char handle[FUN_DIR_HANDLE_SIZE]` (640 bytes) in each `FunWalkFrame`. Platform implementations cast to their internal struct.

**Rationale**: Enables stack allocation of `WalkMem` without heap. `FUN_DIR_HANDLE_SIZE = 640` covers both platforms (Windows: HANDLE + WIN32_FIND_DATAW ≈ 608 bytes; Linux: fd + 256-byte dirent buf ≈ 268 bytes).

### D3: Two path buffers per frame (raw + tokenized)

**Decision**: Each `FunWalkFrame` has `path_raw[]` (untouched C string) and `path_tok[]` (copy that `fun_path_from_string` tokenizes in-place). Component pointers reference `path_tok`.

**Rationale**: `fun_path_from_string` replaces separators with `\0`, destroying the string for use as a C path. `path_raw` is used to open the OS handle and build child paths; `path_tok` is used only for the `Path` struct in `FileEntry`.

### D4: `skip_children` parameter on `walk_next`

**Decision**: The caller passes `skip_children` on each `walk_next` call. If true and the previous entry was a directory, the walker does not descend into it.

**Rationale**: Pruning (not descending) cannot be achieved with just `continue` in the caller loop — the walker would already have queued the descent. The `skip_children` parameter signals the decision before the descent happens. A separate `walk_skip` function was considered but adds API surface for no benefit.

### D5: TSV in-place mutation

**Decision**: `fun_tsv_next` replaces `\t` and `\n` with `\0` directly in the caller's buffer. Fields point into the buffer.

**Rationale**: Consistent with `fun_path_from_string`. No copy needed. Field pointers remain valid for the lifetime of the buffer. Callers who need to retain field values must copy them.

### D6: `fun_filesystem_walk_close` for early exit

**Decision**: Add `fun_filesystem_walk_close(FunWalkState*)` that closes all open handles in the frame stack.

**Rationale**: If the caller exits the walk loop early, open OS handles would otherwise leak until process exit. The close function is a no-op after natural completion (all handles already closed by `walk_next`). This is especially important for long-running processes.

## Risks / Trade-offs

- **16 open file descriptors during deep walks** → Acceptable for CLI tools. Document the limit. Mitigation: `FUN_WALK_MAX_DEPTH = 16` is a hard cap.
- **Handle leak on early exit without `walk_close`** → Documented in API. Bounded by `FUN_WALK_MAX_DEPTH`. Automatic cleanup at process exit covers CLI use cases.
- **Breaking change to `fun_filesystem_list_directory` format** → fundamental-cli callers must be updated in the same change. The new format is a strict superset (just adds a 2-char prefix and tab), making detection straightforward.
- **`FUN_DIR_HANDLE_SIZE = 640` may need updating for future platforms** → Documented constant. Adding a new platform requires verifying the handle struct fits.
- **Entry order not guaranteed** → OS-defined ordering. Not a problem for build tools. Documented as a non-goal.

## Migration Plan

1. Implement TSV module (`include/fundamental/tsv/`, `src/tsv/`)
2. Update both arch `fun_platform_directory_list` implementations to emit TSV prefix
3. Implement walk platform functions in both arch layers
4. Implement `src/filesystem/walk.c`
5. Update `include/fundamental/filesystem/filesystem.h`
6. Update tests; run `code-format.bat`
7. Vendor into `fundamental-cli/`; update `generator.c` and `discovery.c` to use `fun_tsv_next`; rebuild CLI

**Rollback**: revert arch layer changes to remove TSV prefix; remove walk files. No data migration needed.

## Open Questions

None — all decisions resolved during design exploration.
