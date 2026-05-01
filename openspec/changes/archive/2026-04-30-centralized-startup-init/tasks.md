## 1. Header and Types

- [x] 1.1 Create `include/fundamental/startup/startup.h` header file
- [x] 1.2 Define phase constants: `STARTUP_PHASE_PLATFORM`, `STARTUP_PHASE_MEMORY`, etc.
- [x] 1.3 Define `FUNDAMENTAL_STARTUP_REGISTER(phase, function)` macro
- [x] 1.4 Define `FUNDAMENTAL_STARTUP_VERBOSE` config option

## 2. Core Implementation

- [x] 2.1 Update `src/startup/startup.c` with `fun_startup_run()` function
- [x] 2.2 Update `__main()` stub to call `fun_startup_run()`
- [x] 2.3 Implement phase execution loop (phase 1 through 7)
- [x] 2.4 Add error handling with fail-fast behavior
- [x] 2.5 Add verbose tracing (`STARTUP_TRACE()` macro)

## 3. Module Integration: Platform

- [x] 3.1 Add `fun_platform_init()` function (if not exists)
- [x] 3.2 Register platform init at `STARTUP_PHASE_PLATFORM`
- [x] 3.3 Ensure platform init is silent (no logging, no config)

## 4. Module Integration: Config

- [x] 4.1 Add `fun_config_init()` function (if not exists)
- [x] 4.2 Register config init at `STARTUP_PHASE_CONFIG`
- [x] 4.3 Document: config MUST NOT call logging during init
- [x] 4.4 Ensure config loads `fun.ini` silently

## 5. Module Integration: Logging

- [x] 5.1 Update logging module to use centralized `fun_startup_run()` for init
- [x] 5.2 Register logging init at `STARTUP_PHASE_LOGGING`
- [x] 5.3 Logging init reads `[logging]` from already-loaded config
- [x] 5.4 Remove any module-specific `__attribute__((constructor))` from logging

## 6. Module Integration: Network

- [x] 6.1 Update network module to use centralized `fun_startup_run()` for init
- [x] 6.2 Register network init at `STARTUP_PHASE_NETWORK`
- [x] 6.3 Network init reads `[network] rx_buf_size` from config
- [x] 6.4 Remove lazy `pool_init()` call pattern, use centralized init instead

## 7. Testing

- [x] 7.1 Create `tests/startup/` test directory
- [x] 7.2 Create build scripts for Linux and Windows
- [x] 7.3 Write test: startup runs before main()
- [x] 7.4 Write test: phases execute in order
- [x] 7.5 Write test: verbose mode outputs phase names
- [x] 7.6 Write test: platform init failure aborts startup
- [x] 7.7 Write test: config failure uses defaults
- [x] 7.8 Run tests on Linux and verify pass

## 8. Documentation

- [x] 8.1 Document initialization order in header file
- [x] 8.2 Document how modules register init functions
- [x] 8.3 Document circular dependency prevention (config → logging)
- [x] 8.4 Add example: "How to add module to startup sequence"
