## 1. TSV Module

- [x] 1.1 Create `include/fundamental/tsv/tsv.h` — `FunTsvState`, `FunTsvRow`, `FUN_TSV_MAX_FIELDS`, `fun_tsv_init`, `fun_tsv_next` declarations
- [x] 1.2 Create `src/tsv/tsv.c` — implement `fun_tsv_init` (validate, set `_data`/`_pos`/`_len`) and `fun_tsv_next` (scan for `\n`, split on `\t`, skip empty rows)
- [x] 1.3 Create `tests/tsv/test_tsv.c` — tests: single row, multiple rows, empty, extra columns, null args
- [x] 1.4 Create `tests/tsv/build-windows-amd64.bat`
- [x] 1.5 Create `tests/tsv/build-linux-amd64.sh` and mark executable

## 2. Updated Directory Listing Format

- [x] 2.1 Update `arch/filesystem/windows-amd64/directory.c` `fun_platform_directory_list` to emit `D\t` / `F\t` prefix using `dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY`
- [x] 2.2 Update `arch/filesystem/linux-amd64/directory.c` `fun_platform_directory_list` to emit `D\t` / `F\t` prefix using `d->d_type == 4` (DT_DIR)
- [x] 2.3 Update `fun_filesystem_list_directory` doc comment in `include/fundamental/filesystem/filesystem.h` to document TSV format

## 3. Platform Streaming Handle Functions

- [x] 3.1 Add `fun_platform_dir_open`, `fun_platform_dir_read_entry`, `fun_platform_dir_close` to `arch/filesystem/windows-amd64/directory.c` using `FindFirstFileW`/`FindNextFileW`/`FindClose`; internal `WinDirHandle` struct in 640-byte blob
- [x] 3.2 Add same three functions to `arch/filesystem/linux-amd64/directory.c` using `sys_open`/`getdents64`/`sys_close`; internal `LinuxDirHandle` struct with 256-byte dirent buffer in 640-byte blob

## 4. Walk API — Public Header

- [x] 4.1 Add to `include/fundamental/filesystem/filesystem.h`: `FUN_WALK_MAX_DEPTH`, `FUN_WALK_PATH_SIZE`, `FUN_WALK_MAX_COMPONENTS`, `FUN_DIR_HANDLE_SIZE`, `FUN_WALK_MEMORY_SIZE` constants
- [x] 4.2 Add `FileEntry` typedef (path, name, is_directory, depth)
- [x] 4.3 Add `FunWalkState` typedef (_mem, _top, _has_pend)
- [x] 4.4 Add declarations for `fun_filesystem_walk_memory_size`, `fun_filesystem_walk_init`, `fun_filesystem_walk_next`, `fun_filesystem_walk_close`

## 5. Walk API — Implementation

- [x] 5.1 Create `src/filesystem/walk.c` — define `FunWalkFrame` (path_raw, path_tok, components, component_count, is_absolute, handle_open, handle[640]) and `WalkMem` (frames[16], entry/pend path buffers)
- [x] 5.2 Implement `fun_filesystem_walk_memory_size` — returns `FUN_WALK_MEMORY_SIZE`
- [x] 5.3 Implement `fun_filesystem_walk_init` — validate args, zero work_mem, set state fields, convert root path to string, open root handle via `fun_platform_dir_open`
- [x] 5.4 Implement `fun_filesystem_walk_next` — handle pending dir descent, read entry via `fun_platform_dir_read_entry`, build child path, populate `FileEntry`, set pending on directory entries
- [x] 5.5 Implement `fun_filesystem_walk_close` — iterate frames 0.._top, close open handles, reset _top to -1

## 6. Tests

- [x] 6.1 Add walk test setup helper in `tests/filesystem/test_filesystem.c` — creates `test_output/walk_test/` tree (file_a.txt, file_b.txt, sub/file_c.txt, sub/deep/file_d.txt)
- [x] 6.2 Add `test_fun_filesystem_walk_all` — verify 4 files + 2 dirs at correct depths
- [x] 6.3 Add `test_fun_filesystem_walk_skip_children` — skip `sub/`, verify its contents absent
- [x] 6.4 Add `test_fun_filesystem_walk_depth_limit` — skip all dirs at depth >= 1, verify only top-level entries
- [x] 6.5 Add `test_fun_filesystem_walk_empty` — walk empty dir, first call returns false
- [x] 6.6 Add `test_fun_filesystem_walk_close` — init, read one entry, call close, verify no assertion failure
- [x] 6.7 Add `test_fun_filesystem_walk_null` — null args return error from init
- [x] 6.8 Update `test_fun_filesystem_list_directory` to verify TSV format (check `\t` present in output)
- [x] 6.9 Update `tests/filesystem/build-windows-amd64.bat` — add `walk.c` and `tsv.c` to SOURCES
- [x] 6.10 Update `tests/filesystem/build-linux-amd64.sh` — same

## 7. CLAUDE.md and Code Format

- [x] 7.1 Update `CLAUDE.md` — add TSV row to Modules table; update Filesystem row to mention walk functions
- [x] 7.2 Run `code-format.bat` (Windows) / `./code-format.sh` (Linux) on all new and modified files

## 8. fundamental-cli Update (after vendoring)

- [x] 8.1 Run `vendor-fundamental.bat` in `fundamental-cli/` to copy new sources into `vendor/fundamental/`
- [x] 8.2 Update `src/build/generator.c` — replace manual newline-parse loop with `fun_tsv_next`; read type from `row.fields[0]`, name from `row.fields[1]`
- [x] 8.3 Update `src/test/discovery.c` — same update
- [x] 8.4 Update `build-windows-amd64.bat` — add `vendor/fundamental/src/tsv/tsv.c` and `vendor/fundamental/src/filesystem/walk.c`
- [x] 8.5 Update `build-linux-amd64.sh` — same
- [x] 8.6 Build and verify CLI produces a working binary
