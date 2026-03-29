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

- [ ] 3.1 Update `src/platform/platform.c` to register shutdown function via macro
- [ ] 3.2 Implement `platform_deinit()` function for platform-specific shutdown cleanup
- [ ] 3.3 Register at `SHUTDOWN_PHASE_PLATFORM`

## 4. Module Integration: Memory

- [ ] 4.1 Update `src/memory/memory.c` to register shutdown function via macro
- [ ] 4.2 Implement `memory_deinit()` function to clean up any persistent state
- [ ] 4.3 Register at `SHUTDOWN_PHASE_MEMORY`

## 5. Module Integration: Filesystem

- [ ] 5.1 Update `src/filesystem/filesystem.c` to register shutdown function via macro
- [ ] 5.2 Implement `filesystem_deinit()` function to flush any pending operations
- [ ] 5.3 Register at `SHUTDOWN_PHASE_FILESYSTEM`

## 6. Module Integration: Config

- [ ] 6.1 Update `src/config/config.c` to register shutdown function via macro
- [ ] 6.2 Implement `config_deinit()` function to release config state
- [ ] 6.3 Register at `SHUTDOWN_PHASE_CONFIG`

## 7. Signal/Event Implementation

- [ ] 7.1 Create `arch/signals/linux-amd64/signal.c` for Linux signal handling
- [ ] 7.2 Create `arch/signals/windows-amd64/signal.c` for Windows console handler
- [ ] 7.3 Implement SIGTERM handler to call `fun_shutdown_run(SHUTDOWN_EXTERNAL, 0)`
- [ ] 7.4 Implement SIGINT handler to call `fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)`
- [ ] 7.5 Implement Windows Ctrl+C handler to call `fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)`
- [ ] 7.6 Install signal handlers at startup time (in `fun_startup_run()`)

## 8. Integration with Startup Framework

- [ ] 8.1 Update `src/startup/startup.c` to register signal handler installation
- [ ] 8.2 Ensure startup framework calls `install_signal_handlers()` upon startup
- [ ] 8.3 Verify proper cleanup registration flow from startup to shutdown

## 9. Testing

- [ ] 9.1 Create `tests/shutdown/` test directory
- [ ] 9.2 Create build scripts for Linux and Windows
- [ ] 9.3 Write test: shutdown runs registered cleanup functions
- [ ] 9.4 Write test: shutdown is idempotent (safe to call multiple times)
- [ ] 9.5 Write test: atomic operations prevent race conditions
- [ ] 9.6 Write test: shutdown executes in reverse order of initialization
- [ ] 9.7 Write test: signal handlers trigger proper shutdown sequence
- [ ] 9.8 Run tests on Linux and verify all pass

## 10. Documentation

- [ ] 10.1 Document shutdown function API in header file  
- [ ] 10.2 Document phase order and timing guarantees
- [ ] 10.3 Add example: "How to register application cleanup function"
- [ ] 10.4 Update main README with shutdown framework capabilities
