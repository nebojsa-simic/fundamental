## Context

File renaming is a common task that demonstrates filesystem operations, string manipulation, and user interaction. The Fundamental Library has all the necessary primitives (directory listing, path operations, string functions) but needs a practical demonstration that combines them. This demo will show safe file operations with preview capability.

**Constraints:**
- Must follow existing demo patterns (build scripts for Windows/Linux)
- Must use only Fundamental Library APIs
- Must handle errors gracefully (permission denied, name conflicts, etc.)
- Must work on both Windows and Linux with platform-specific path separators
- Should demonstrate dry-run mode for safety

## Goals / Non-Goals

**Goals:**
- List files in a directory matching wildcard patterns
- Support multiple renaming modes: find/replace, prefix/suffix, numbering, extension change
- Preview changes before applying (dry-run mode)
- Handle errors gracefully with clear messages
- Demonstrate filesystem enumeration and path operations
- Show string manipulation (replacement, concatenation)
- Use arrays for file list storage

**Non-Goals:**
- Recursive directory traversal (subdirectories)
- Undo/rollback functionality
- GUI or interactive TUI
- Complex regex-based renaming
- File content-based renaming
- Backup creation before renaming

## Decisions

### 1. File Selection Strategy
**Decision:** Use `fun_filesystem_list_directory()` with client-side wildcard filtering

**Rationale:**
- Demonstrates directory listing functionality
- Wildcard matching implemented with string operations
- Single directory (non-recursive) keeps demo simple
- Client-side filtering shows string comparison operations

**Alternatives considered:**
- Recursive walk: Adds complexity, can demonstrate separately
- Platform-specific find APIs: Less portable, more complex

### 2. Wildcard Pattern Matching
**Decision:** Implement simple glob-style matching (* and ?) using string operations

**Rationale:**
- Common pattern users understand
- Demonstrates string comparison and iteration
- No regex engine needed
- Can be implemented with fundamental string ops

**Pattern support:**
- `*` matches zero or more characters
- `?` matches exactly one character
- No character classes or complex patterns

### 3. Renaming Modes
**Decision:** Support four distinct modes via command-line flags

**Modes:**
1. `--find-replace "old" "new"` - Replace substring in filename
2. `--prefix "pre_" --suffix "_suf"` - Add text before/after name
3. `--number` - Add sequential numbers (file_001.txt, file_002.txt)
4. `--extension ".new"` - Change file extension

**Rationale:**
- Covers 90% of batch renaming use cases
- Each mode demonstrates different string operations
- Can be combined (e.g., prefix + numbering)
- Clear separation of concerns

### 4. Dry-Run Default
**Decision:** Default to dry-run mode, require `--apply` flag for actual renaming

**Rationale:**
- Safe by default - prevents accidental data loss
- Demonstrates preview pattern
- Users can see what will happen before committing
- Follows principle of least surprise

**Alternatives considered:**
- Require explicit `--dry-run`: Risky, users might forget
- Interactive confirmation: More complex I/O

### 5. Numbering Format
**Decision:** Use zero-padded numbers with configurable width (default 3 digits)

**Format:** `basename_001.ext`, `basename_002.ext`, etc.

**Rationale:**
- Zero-padding maintains sort order
- 3 digits sufficient for most cases (<1000 files)
- Demonstrates integer-to-string conversion
- Can add `--number-width` flag if needed

### 6. Conflict Handling
**Decision:** Check for existing files before renaming, report conflicts in preview

**Rationale:**
- Prevents accidental overwrites
- Demonstrates `fun_file_exists()` check
- Clear error reporting
- Skip conflicting files in apply mode

**Alternatives considered:**
- Auto-rename conflicts: Complex, unpredictable
- Overwrite silently: Dangerous
- Prompt interactively: Complex I/O

### 7. Path Handling
**Decision:** Use Fundamental Library path utilities for all path operations

**Rationale:**
- Cross-platform compatibility (Windows/Linux)
- Demonstrates `fun_path_*` functions
- Handles path separators automatically
- Proper path joining and normalization

## Risks / Trade-offs

**[Risk] Accidental file overwrites if user skips preview**
→ Mitigation: Default to dry-run, clear warnings in output

**[Risk] Race conditions if files change during operation**
→ Mitigation: Accept limitation for demo, mention in docs

**[Risk] Wildcard patterns may be confusing**
→ Mitigation: Provide clear examples in help text

**[Risk] Numbering order depends on directory listing order**
→ Mitigation: Document that order is filesystem-dependent, suggest sorting if needed

**[Trade-off] Simplicity vs. features**
→ Decision: Favor simplicity. Production version could add undo, regex, recursive, etc.

**[Trade-off] Performance vs. safety checks**
→ Decision: Check every file exists before renaming, even if slower

## Migration Plan

Not applicable - this is a new demo application with no migration required.

## Open Questions

None - the design is straightforward and uses existing library features.
