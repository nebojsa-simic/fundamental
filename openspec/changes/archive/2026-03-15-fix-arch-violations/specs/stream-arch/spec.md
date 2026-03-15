## ADDED Requirements

### Requirement: Arch layer provides stream handle close
Each platform arch implementation SHALL provide `arch_stream_close_handle(void *internal_state)` which closes the platform-specific OS handle held in the arch-internal state struct. The `src/stream/streamLifecycle.c` module SHALL call this function and SHALL NOT include any arch-specific header directly or reference any OS handle types (`INVALID_HANDLE_VALUE`, `HANDLE`, file descriptors, etc.).

#### Scenario: Windows implementation closes HANDLE
- **WHEN** `arch_stream_close_handle(state)` is called on a Windows AMD64 build with a valid open handle
- **THEN** it calls `CloseHandle()` on the file handle if it is not `INVALID_HANDLE_VALUE`

#### Scenario: Linux implementation closes file descriptor
- **WHEN** `arch_stream_close_handle(state)` is called on a Linux AMD64 build with a valid open file descriptor
- **THEN** it issues the `close` syscall on the descriptor if it is not `-1`

#### Scenario: src/stream/streamLifecycle.c contains no arch-specific includes
- **WHEN** `src/stream/streamLifecycle.c` is inspected
- **THEN** it contains no direct include of any arch/ header path and no reference to Windows or Linux OS types
