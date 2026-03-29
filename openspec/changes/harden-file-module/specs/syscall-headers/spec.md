## ADDED Requirements

### Requirement: Syscall Header Include

All files using syscalls SHALL include `<sys/syscall.h>`.

#### Scenario: Header inclusion
- **WHEN** file uses syscall functions
- **THEN** `#include <sys/syscall.h>` is present

### Requirement: Remove Hardcoded Syscall Numbers

Hardcoded syscall number definitions SHALL be removed.

#### Scenario: SYS_open
- **WHEN** `fileReadMmap.c` is examined
- **THEN** `#define SYS_open 2` does NOT exist

#### Scenario: SYS_close
- **WHEN** file modules are examined
- **THEN** `#define SYS_close 3` does NOT exist

#### Scenario: SYS_mmap
- **WHEN** file modules are examined
- **THEN** `#define SYS_mmap 9` does NOT exist

### Requirement: Standard Syscall Constants

Syscall constants SHALL be used from the standard header.

#### Scenario: SYS_open usage
- **WHEN** `syscall2(SYS_open, ...)` is called
- **THEN** `SYS_open` comes from `<sys/syscall.h>`

#### Scenario: SYS_close usage
- **WHEN** `syscall1(SYS_close, ...)` is called
- **THEN** `SYS_close` comes from `<sys/syscall.h>`

### Requirement: Architecture Portability

The implementation SHALL compile correctly on different architectures.

#### Scenario: x86_64
- **WHEN** compiled on x86_64
- **THEN** correct syscall numbers are used from header

#### Scenario: ARM64
- **WHEN** compiled on ARM64
- **THEN** correct syscall numbers are used from header (different from x86_64)

#### Scenario: RISC-V
- **WHEN** compiled on RISC-V
- **THEN** correct syscall numbers are used from header (different from x86_64 and ARM64)

### Requirement: Affected Files

All file module implementation files SHALL use syscall headers.

#### Scenario: fileReadMmap.c
- **WHEN** `fileReadMmap.c` is compiled
- **THEN** it includes `<sys/syscall.h>` and has no hardcoded syscall numbers

#### Scenario: fileWriteMmap.c
- **WHEN** `fileWriteMmap.c` is compiled
- **THEN** it includes `<sys/syscall.h>` and has no hardcoded syscall numbers

#### Scenario: fileAppend.c
- **WHEN** `fileAppend.c` is compiled
- **THEN** it includes `<sys/syscall.h>` and has no hardcoded syscall numbers

#### Scenario: fileLock.c
- **WHEN** `fileLock.c` is compiled
- **THEN** it includes `<sys/syscall.h>` and has no hardcoded syscall numbers

#### Scenario: fileNotification.c
- **WHEN** `fileNotification.c` is compiled
- **THEN** it includes `<sys/syscall.h>` and has no hardcoded syscall numbers
