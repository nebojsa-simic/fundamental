## MODIFIED Requirements

### Requirement: Walk Entry Iteration
The filesystem module SHALL yield one `FileEntry` per call containing the full path, name, type, and depth of each entry in the tree. Entries whose full path string exceeds `FUN_WALK_PATH_SIZE` bytes are silently skipped.

#### Scenario: Advance to next entry
- **WHEN** `fun_filesystem_walk_next(FunWalkState *state, FileEntry *entry, bool skip_children)` is called and entries remain
- **THEN** `entry->path` contains the full path to the entry
- **AND** `entry->name` is the filename component of the entry
- **AND** `entry->is_directory` is true if the entry is a directory
- **AND** `entry->depth` is 0 for direct children of root and increments by 1 per level
- **AND** the return value is true

#### Scenario: Walk complete
- **WHEN** `fun_filesystem_walk_next` is called and all entries have been yielded
- **THEN** the return value is false
- **AND** all directory handles are closed

#### Scenario: Entry validity
- **WHEN** a `FileEntry` is returned by `fun_filesystem_walk_next`
- **THEN** `entry->path.components` and `entry->name` are valid only until the next call to `fun_filesystem_walk_next`

#### Scenario: Root directory not yielded
- **WHEN** `fun_filesystem_walk_init` completes successfully
- **THEN** the root directory itself is NOT yielded as an entry; only its contents are

#### Scenario: Entry path exceeds walk path buffer
- **WHEN** an entry's full path string would exceed `FUN_WALK_PATH_SIZE` bytes
- **THEN** that entry is skipped and not yielded to the caller
- **AND** the walk continues with the next entry
- **AND** the return value from `fun_filesystem_walk_next` reflects the next available entry, not the skipped one
