## ADDED Requirements

### Requirement: Durability Mode Enum

The file module SHALL define a durability mode enumeration.

#### Scenario: FileDurabilityMode enum
- **WHEN** durability modes are defined
- **THEN** enum contains `FILE_DURABILITY_ASYNC`, `FILE_DURABILITY_SYNC`, `FILE_DURABILITY_FULL`

### Requirement: Durability Mode Parameter

Write and append operations SHALL accept a durability mode parameter.

#### Scenario: FileWriteParameters
- **WHEN** `FileWriteParameters` is defined
- **THEN** it contains `durability_mode` field of type `FileDurabilityMode`

#### Scenario: FileAppendParameters
- **WHEN** `FileAppendParameters` is defined
- **THEN** it contains `durability_mode` field of type `FileDurabilityMode`

### Requirement: ASYNC Mode (Default)

ASYNC mode SHALL write data to page cache without explicit sync.

#### Scenario: ASYNC durability
- **WHEN** `durability_mode == FILE_DURABILITY_ASYNC`
- **THEN** no `msync()` or `fsync()` is called after write
- **AND** data may be lost on crash (page cache only)

### Requirement: SYNC Mode

SYNC mode SHALL call `msync(MS_SYNC)` after mmap write.

#### Scenario: SYNC durability with mmap
- **WHEN** `durability_mode == FILE_DURABILITY_SYNC` and mmap is used
- **THEN** `syscall3(SYS_msync, address, size, MS_SYNC)` is called after write
- **AND** data is guaranteed on disk after call returns

### Requirement: FULL Mode

FULL mode SHALL call `fsync()` after write.

#### Scenario: FULL durability
- **WHEN** `durability_mode == FILE_DURABILITY_FULL`
- **THEN** `syscall1(SYS_fsync, file_descriptor)` is called after write
- **AND** data and metadata are guaranteed on disk

### Requirement: Default Mode

The default durability mode SHALL be `FILE_DURABILITY_ASYNC` for backward compatibility.

#### Scenario: Unspecified mode
- **WHEN** durability mode is not explicitly set
- **THEN** `FILE_DURABILITY_ASYNC` is used

### Requirement: Error Handling

Sync operation failures SHALL be reported as errors.

#### Scenario: msync failure
- **WHEN** `msync()` returns error
- **THEN** operation returns `ASYNC_ERROR` with appropriate error code

#### Scenario: fsync failure
- **WHEN** `fsync()` returns error
- **THEN** operation returns `ASYNC_ERROR` with appropriate error code

### Requirement: Performance Documentation

The performance implications of each durability mode SHALL be documented.

#### Scenario: Documentation
- **WHEN** developer reads header file
- **THEN** performance characteristics of each mode are documented
- **AND** ASYNC is fastest, FULL is slowest but safest
