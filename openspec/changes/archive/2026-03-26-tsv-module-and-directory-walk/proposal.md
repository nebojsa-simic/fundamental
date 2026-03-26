## Why

The `fun_filesystem_list_directory` function returns opaque name-only strings, forcing every caller to make extra syscalls to determine entry types and re-implement the same parsing boilerplate. The fundamental-cli has two independent copies of this logic already. Providing a TSV listing format and a streaming walk iterator in the library eliminates this duplication and correctly surfaces type info that the OS already delivers for free at listing time.

## What Changes

- **New capability**: `tsv` — streaming TSV parser (`fun_tsv_init`, `fun_tsv_next`) that tokenizes a mutable buffer in-place with no heap allocation
- **New capability**: `filesystem-walk` — streaming directory walk iterator (`fun_filesystem_walk_init`, `fun_filesystem_walk_next`, `fun_filesystem_walk_close`) using per-frame OS handles for true entry-by-entry streaming with no listing buffer and no truncation risk
- **BREAKING**: `fun_filesystem_list_directory` output format changes from `name\n` to `D\tname\n` / `F\tname\n` TSV — callers must be updated to parse the type prefix

## Capabilities

### New Capabilities

- `tsv`: Streaming TSV row parser. `fun_tsv_init` takes a mutable null-terminated buffer (modified in-place). `fun_tsv_next` yields one `FunTsvRow` at a time with a `fields[]` pointer array. State is ~280 bytes, stack-allocatable, no work memory needed.
- `filesystem-walk`: Streaming directory walk over a path tree. `fun_filesystem_walk_init` opens the root directory handle into caller-provided work memory (40KB fixed). `fun_filesystem_walk_next` yields one `FileEntry` (path, name, is_directory, depth) per call, reading directly from the OS. `fun_filesystem_walk_close` releases open handles on early exit.

### Modified Capabilities

- `filesystem`: `fun_filesystem_list_directory` output format changes from plain `name\n` to TSV `D\tname\n` / `F\tname\n`. The function signature is unchanged but callers must now parse the type prefix. Both arch implementations gain three new platform streaming functions (`fun_platform_dir_open`, `fun_platform_dir_read_entry`, `fun_platform_dir_close`) used by the walk implementation.

## Impact

- **`include/fundamental/tsv/tsv.h`**, **`src/tsv/tsv.c`** — new TSV module
- **`include/fundamental/filesystem/filesystem.h`** — new `FileEntry`, `FunWalkState`, walk constants and declarations; updated `fun_filesystem_list_directory` doc
- **`src/filesystem/walk.c`** — new walk implementation
- **`arch/filesystem/windows-amd64/directory.c`**, **`arch/filesystem/linux-amd64/directory.c`** — TSV prefix added to listing output; new streaming handle functions added
- **`fundamental-cli/` `src/build/generator.c`**, **`src/test/discovery.c`** — must be updated to parse the new TSV listing format
