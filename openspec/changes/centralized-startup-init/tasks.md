## 1. Header and Types

- [ ] 1.1 Create `include/fundamental/startup/startup.h` header file
- [ ] 1.2 Define phase constants: `STARTUP_PHASE_PLATFORM`, `STARTUP_PHASE_MEMORY`, etc.
- [ ] 1.3 Define `FUNDAMENTAL_STARTUP_REGISTER(phase, function)` macro
- [ ] 1.4 Define `FUNDAMENTAL_STARTUP_VERBOSE` config option

## 2. Core Implementation

- [ ] 2.1 Update `src/startup/startup.c` with `fun_startup_run()` function
- [ ] 2.2 Update `__main()` stub to call `fun_startup_run()`
- [ ] 2.3 Implement phase execution loop (phase 1 through 7)
- [ ] 2.4 Add error handling with fail-fast behavior
- [ ] 2.5 Add verbose tracing (`STARTUP_TRACE()` macro)

## 3. Module Integration: Platform

- [ ] 3.1 Add `fun_platform_init()` function (if not exists)
- [ ] 3.2 Register platform init at `STARTUP_PHASE_PLATFORM`
- [ ] 3.3 Ensure platform init is silent (no logging, no config)

## 4. Module Integration: Config

- [ ] 4.1 Add `fun_config_init()` function (if not exists)
- [ ] 4.2 Register config init at `STARTUP_PHASE_CONFIG`
- [ ] 4.3 Document: config MUST NOT call logging during init
- [ ] 4.4 Ensure config loads `fun.ini` silently

## 5. Module Integration: Logging

- [ ] 5.1 Update logging module to use centralized `fun_startup_run()` for init
- [ ] 5.2 Register logging init at `STARTUP_PHASE_LOGGING`
- [ ] 5.3 Logging init reads `[logging]` from already-loaded config
- [ ] 5.4 Remove any module-specific `__attribute__((constructor))` from logging

## 6. Module Integration: Network

- [ ] 6.1 Update network module to use centralized `fun_startup_run()` for init
- [ ] 6.2 Register network init at `STARTUP_PHASE_NETWORK`
- [ ] 6.3 Network init reads `[network] rx_buf_size` from config
- [ ] 6.4 Remove lazy `pool_init()` call pattern, use centralized init instead

## 7. Testing

- [ ] 7.1 Create `tests/startup/` test directory
- [ ] 7.2 Create build scripts for Linux and Windows
- [ ] 7.3 Write test: startup runs before main()
- [ ] 7.4 Write test: phases execute in order
- [ ] 7.5 Write test: verbose mode outputs phase names
- [ ] 7.6 Write test: platform init failure aborts startup
- [ ] 7.7 Write test: config failure uses defaults
- [ ] 7.8 Run tests on Linux and verify pass

## 8. Documentation

- [ ] 8.1 Document initialization order in header file
- [ ] 8.2 Document how modules register init functions
- [ ] 8.3 Document circular dependency prevention (config → logging)
- [ ] 8.4 Add example: "How to add module to startup sequence"
