---
name: Config module QA status
description: QA findings for the add-config-module implementation — test commands, known issues, and accepted pre-existing warnings
type: project
---

Config module was implemented and verified clean on 2026-03-14. README.md documentation updated 2026-03-14 (task 21.8). Full suite re-run confirmed clean on 2026-03-14 after README changes — ready to archive.

**Test command:** `pwsh -Command "& C:\Users\nsimi\workspaces-win\fundamental\tests\config\build-windows-amd64.bat"` then `pwsh -Command "& C:\Users\nsimi\workspaces-win\fundamental\tests\config\test.exe"`
**Full suite:** `pwsh -Command "& C:\Users\nsimi\workspaces-win\fundamental\run-tests-windows-amd64.bat"`
**Result:** 36/36 config tests pass. Full suite passes with no new failures.

**Pre-existing warnings (not caused by config module):**
- hashmap.c: `fun_memory_copy` discards `const` qualifier (3 occurrences) — pre-existing
- rbtree.h: `ERROR_CODE_KEY_NOT_FOUND 302` and `ERROR_CODE_KEY_EXISTS 301` overflow `uint8_t` — pre-existing bug in rbtree (302 truncates to 46, 301 to 45). Tests still pass because tests check values that work with the truncated codes, but this is a real bug in the rbtree module, not in config.
- stringOperations.c: unused variable `original_begin` — pre-existing
- filesystem/directory.c: unused functions `add_entry_to_buffer`, `find_last_separator` — pre-existing
- file_exists.c: unused `fun_wide_string_length` — pre-existing
- test_config.c: unused `write_ini_file`, `remove_ini_file` — in test file only, scaffolding left over from planned INI file tests that use CLI path instead

**Why:** Config module is In Dev status. Full INI-file-based testing requires a real INI file at the exe directory, which tests work around by using CLI args to exercise the same storage path.
**How to apply:** When retesting config or rbtree, expect these warnings. The rbtree uint8_t overflow is a latent bug to watch if rbtree error codes are ever checked by value.
