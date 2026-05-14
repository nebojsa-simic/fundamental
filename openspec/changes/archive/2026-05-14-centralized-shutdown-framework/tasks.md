## 1. Header and Types

- [x] 1.1 Create `include/fundamental/shutdown/shutdown.h` header file
- [x] 1.2 Define shutdown type enum: `SHUTDOWN_NORMAL`, `SHUTDOWN_ABNORMAL`, `SHUTDOWN_EXTERNAL`, `SHUTDOWN_EMERGENCY`
- [x] 1.3 Define phase constants: `SHUTDOWN_PHASE_PLATFORM`, `SHUTDOWN_PHASE_MEMORY`, etc., `SHUTDOWN_PHASE_APP = 99`
- [x] 1.4 Define `FUNDAMENTAL_SHUTDOWN_REGISTER(phase, function)` macro
- [x] 1.5 Declare `fun_shutdown_run()` function with exit code parameter

## 2. Core Implementation

- [x] 2.1 Create `src/shutdown/shutdown.c` with `fun_shutdown_run()` function
- [x] 2.2 Implement atomic shutdown state tracking (`shutdown_started`, `shutdown_completed`)
- [x] 2.3 Add atomic Compare-And-Swap to prevent concurrent execution of shutdown sequence
- [x] 2.4 Implement phase execution loop (APP, NETWORK, FILE, MEMORY, PLATFORM in reverse order)
- [x] 2.5 Pass exit_code parameter through to shutdown handlers

## 3. Module Integration: Platform

- [x] 3.1 Update `src/platform/platform.c` to register shutdown function via macro
- [x] 3.2 Implement `platform_deinit()` function for platform-specific shutdown cleanup
- [x] 3.3 Register at `SHUTDOWN_PHASE_PLATFORM`

## 4. Module Integration: Memory

- [x] 4.1 Update `src/startup/startup.c` to register shutdown function via macro
- [x] 4.2 Implement `memory_deinit()` function to clean up any persistent state
- [x] 4.3 Register at `SHUTDOWN_PHASE_MEMORY`

## 5. Module Integration: Filesystem

- [x] 5.1 Update `src/filesystem/path.c` to register shutdown function via macro
- [x] 5.2 Implement `filesystem_deinit()` function to flush any pending operations
- [x] 5.3 Register at `SHUTDOWN_PHASE_FILESYSTEM`

## 6. Module Integration: Config

- [x] 6.1 Update `src/config/config.c` to register shutdown function via macro
- [x] 6.2 Implement `config_deinit()` function to release config state
- [x] 6.3 Register at `SHUTDOWN_PHASE_CONFIG`

## 7. Signal/Event Implementation

- [x] 7.1 Create `arch/signals/linux-amd64/signal.c` for Linux signal handling
- [x] 7.2 Create `arch/signals/windows-amd64/signal.c` for Windows console handler
- [x] 7.3 Implement SIGTERM handler to call `fun_shutdown_run(SHUTDOWN_EXTERNAL, 0)`
- [x] 7.4 Implement SIGINT handler to call `fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)`
- [x] 7.5 Implement Windows Ctrl+C handler to call `fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)`
- [x] 7.6 Install signal handlers at startup time (in `fun_startup_run()`)

## 8. Integration with Startup Framework

- [x] 8.1 Update `src/startup/startup.c` to register signal handler installation
- [x] 8.2 Ensure startup framework calls `install_signal_handlers()` upon startup
- [x] 8.3 Verify proper cleanup registration flow from startup to shutdown

## 9. Testing

- [x] 9.1 Create `tests/shutdown/` test directory
- [x] 9.2 Create build scripts for Linux and Windows
- [x] 9.3 Write test: shutdown runs registered cleanup functions
- [x] 9.4 Write test: shutdown is idempotent (safe to call multiple times)
- [x] 9.5 Write test: atomic operations prevent race conditions
- [x] 9.6 Write test: shutdown executes in reverse order of initialization
- [x] 9.7 Write test: signal handlers trigger proper shutdown sequence
- [x] 9.8 Run tests on Linux and verify all pass

## 10. Documentation

- [x] 10.1 Document shutdown function API in header file  
- [x] 10.2 Document phase order and timing guarantees
- [x] 10.3 Add example: "How to register application cleanup function"
- [x] 10.4 Update main README with shutdown framework capabilities
