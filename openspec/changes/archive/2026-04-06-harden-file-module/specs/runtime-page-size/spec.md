## ADDED Requirements

### Requirement: Runtime Page Size Detection

The file module SHALL detect page size at runtime using `sysconf()`.

#### Scenario: sysconf call
- **WHEN** page size is needed
- **THEN** `sysconf(_SC_PAGESIZE)` is called

#### Scenario: sysconf success
- **WHEN** `sysconf(_SC_PAGESIZE)` returns positive value
- **THEN** that value is used as page size

#### Scenario: sysconf failure
- **WHEN** `sysconf(_SC_PAGESIZE)` returns -1
- **THEN** fallback page size of 4096 is used

### Requirement: Page Size Caching

Page size SHALL be cached after first detection to avoid repeated syscalls.

#### Scenario: First call
- **WHEN** `get_page_size()` is called first time
- **THEN** `sysconf()` is called and result is cached

#### Scenario: Subsequent calls
- **WHEN** `get_page_size()` is called after first call
- **THEN** cached value is returned (no syscall)

### Requirement: Static Cache

The page size cache SHALL be stored in a static variable.

#### Scenario: Cache persistence
- **WHEN** `get_page_size()` is called from different functions
- **THEN** same cached value is returned

### Requirement: Removal of Hardcoded Page Size

Hardcoded `#define PAGE_SIZE 4096` SHALL be removed from all files.

#### Scenario: fileReadMmap.c
- **WHEN** `fileReadMmap.c` is compiled
- **THEN** no `#define PAGE_SIZE 4096` exists

#### Scenario: fileWriteMmap.c
- **WHEN** `fileWriteMmap.c` is compiled
- **THEN** no `#define PAGE_SIZE 4096` exists

### Requirement: Offset Alignment

File offsets SHALL be aligned to runtime-detected page size.

#### Scenario: Offset alignment
- **WHEN** calculating `aligned_offset`
- **THEN** `offset & ~(page_size - 1)` uses runtime page size

### Requirement: Portability

The implementation SHALL work correctly on architectures with different page sizes.

#### Scenario: x86_64 (4KB pages)
- **WHEN** running on x86_64
- **THEN** page size is 4096

#### Scenario: ARM64 (may have 64KB pages)
- **WHEN** running on ARM64 with 64KB pages
- **THEN** page size is 65536

#### Scenario: RISC-V (may have variable pages)
- **WHEN** running on RISC-V
- **THEN** correct page size is detected
