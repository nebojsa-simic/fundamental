### Requirement: Walk Initialisation
The filesystem module SHALL initialise a streaming directory walk using caller-provided work memory, opening the root directory handle immediately.

#### Scenario: Successful initialisation
- **WHEN** `fun_filesystem_walk_init(FunWalkState *state, Memory work_mem, Path root)` is called with valid arguments and work_mem of at least `fun_filesystem_walk_memory_size()` bytes
- **THEN** the walker is ready to yield entries
- **AND** the root directory handle is open

#### Scenario: Root directory does not exist
- **WHEN** `fun_filesystem_walk_init` is called with a root path that does not exist
- **THEN** an error result is returned

#### Scenario: Null arguments
- **WHEN** `fun_filesystem_walk_init` is called with a null state or null work_mem
- **THEN** an error result is returned

### Requirement: Walk Entry Iteration
The filesystem module SHALL yield one `FileEntry` per call containing the full path, name, type, and depth of each entry in the tree.

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

### Requirement: Walk Pruning
The filesystem module SHALL allow the caller to skip descent into a directory by passing `skip_children=true`.

#### Scenario: Skip directory descent
- **WHEN** `fun_filesystem_walk_next` is called with `skip_children=true` and the previously yielded entry was a directory
- **THEN** the walker does not descend into that directory
- **AND** the next entry is from the same or a parent level

#### Scenario: Skip has no effect on files
- **WHEN** `fun_filesystem_walk_next` is called with `skip_children=true` and the previously yielded entry was a file
- **THEN** `skip_children` is ignored

### Requirement: Walk Depth Limit
The filesystem module SHALL limit walk depth to `FUN_WALK_MAX_DEPTH` levels.

#### Scenario: Depth cap enforced
- **WHEN** the directory tree is deeper than `FUN_WALK_MAX_DEPTH`
- **THEN** entries beyond `FUN_WALK_MAX_DEPTH` are not yielded

### Requirement: Walk Early Exit
The filesystem module SHALL provide a close function to release open directory handles when a walk is abandoned before completion.

#### Scenario: Close after early exit
- **WHEN** `fun_filesystem_walk_close(FunWalkState *state)` is called before the walk is exhausted
- **THEN** all open directory handles are released

#### Scenario: Close after natural completion
- **WHEN** `fun_filesystem_walk_close` is called after `fun_filesystem_walk_next` has returned false
- **THEN** the call is a no-op (handles were already closed)

### Requirement: Walk Memory
The filesystem module SHALL use a fixed-size caller-provided work buffer with no internal heap allocation.

#### Scenario: Work memory size
- **WHEN** `fun_filesystem_walk_memory_size()` is called
- **THEN** it returns the exact number of bytes required for work_mem

#### Scenario: No truncation for any directory size
- **WHEN** a directory contains any number of entries
- **THEN** all entries are yielded regardless of total entry size
- **AND** there is no fixed listing buffer that can overflow
