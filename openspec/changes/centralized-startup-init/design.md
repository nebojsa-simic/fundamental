## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    STARTUP INITIALIZATION                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  __main() stub [src/startup/startup.c]                          │
│  (called by GCC-generated code at program startup)              │
│         │                                                        │
│         ▼                                                        │
│  fun_startup_run()                                               │
│         │                                                        │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  Phase 1: Platform (CPU detection, basic init)          │    │
│  │  Phase 2: Memory (allocator init)                        │    │
│  │  Phase 3: Filesystem (path separators, etc.)            │    │
│  │  Phase 4: Config (load fun.ini)                          │    │
│  │  Phase 5: Logging (load [logging] config)               │    │
│  │  Phase 6: Network (pool init, rx_buf_size config)       │    │
│  │  Phase 7: Other modules...                               │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                  │
│  MODULE REGISTRATION:                                            │
│  ─────────────────────                                           │
│  // In logging.c                                                 │
│  static void logging_init(void) { ... }                          │
│  FUNDAMENTAL_STARTUP_REGISTER(STARTUP_PHASE_LOGGING, logging_init) │
│                                                                  │
│  ─────────────────────────────────────────────────────────────   │
│  Startup phases ensure logging runs AFTER config is loaded.      │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

## Design Decisions

### 1. Centralized Dispatcher via __main()

**Decision**: Replace `__main()` stub with call to `fun_startup_run()` which executes all module initializers in defined order.

**Rationale**:
- Uses existing GCC-generated `__main()` call mechanism
- No GCC constructor attributes needed
- Controlled initialization order
- Single place to debug startup issues
- Visible list of what initializes
- Easy to add new phases

**Implementation**:
```c
// src/startup/startup.c
void __main(void)
{
    fun_startup_run();
}

void fun_startup_run(void)
{
    // Phase 1: Platform
    fun_platform_init();
    
    // Phase 2: Memory
    fun_memory_init();
    
    // Phase 3: Filesystem
    fun_filesystem_init();
    
    // Phase 4: Config
    fun_config_init();
    
    // Phase 5: Logging
    fun_logging_init();
    
    // Phase 6: Network
    fun_network_init();
}
```

---

### 2. Module Registration via Macros

**Decision**: Modules register init functions via `FUNDAMENTAL_STARTUP_REGISTER(phase, function)` macro.

**Rationale**:
- Modules declare their own initialization requirements
- Header documents what each module needs
- Easy to see dependencies

**Implementation**:
```c
// include/fundamental/startup/startup.h
#define STARTUP_PHASE_PLATFORM  1
#define STARTUP_PHASE_MEMORY    2
#define STARTUP_PHASE_FILESYSTEM 3
#define STARTUP_PHASE_CONFIG    4
#define STARTUP_PHASE_LOGGING   5
#define STARTUP_PHASE_NETWORK   6

// In logging.c
#include "fundamental/startup/startup.h"

static void logging_init(void) {
    // Config is already loaded at this point
    fun_config_load(...);
}

FUNDAMENTAL_STARTUP_REGISTER(STARTUP_PHASE_LOGGING, logging_init);
```

---

### 3. No Dynamic Registration (Static Array)

**Decision**: Init functions stored in static array at compile time, not dynamic registration.

**Rationale**:
- No dynamic memory allocation during startup
- No registration order race conditions
- Smaller code footprint
- All init functions known at compile time

**Implementation**:
```c
typedef void (*startup_fn)(void);

static startup_fn phase_logging[] = {
    logging_init,
    NULL
};
```

---

### 4. Circular Dependency Prevention: Config → Logging

**Decision**: Config module MUST NOT call logging. Logging initializes after config.

**Enforcement**:
- Document in both modules
- Config init at phase 4, logging at phase 5
- If config needs diagnostics, use raw console output (not logging module)

**Pattern**:
```c
// config.c - DO NOT DO THIS
#include "fundamental/logging/logging.h"  // ← Circular dependency!

void config_init(void) {
    log_info("Loading config...");  // ← Would recurse infinitely
}

// Correct: configinit uses no logging
void config_init(void) {
    // Silent initialization
    // If debug needed, use: fun_console_write_line("Config loading...");
}
```

---

### 5. Error Handling: Fail Fast

**Decision**: If initialization fails, abort immediately with diagnostic.

**Rationale**:
- Partial initialization is worse than no initialization
- Silent failures lead to confusing crashes later
- Better to know immediately what went wrong

**Implementation**:
```c
void fun_startup_run(void)
{
    if (fun_platform_init() != 0) {
        fun_console_error_line("FATAL: Platform init failed");
        __builtin_trap();  // Or exit(1) if available
    }
    // ... continue for other phases
}
```

---

### 6. Optional Startup Diagnostics

**Decision**: Provide compile-time flag to print initialization sequence.

**Rationale**:
- Debug which phase is slow
- Verify init order
- Diagnose startup crashes

**Implementation**:
```c
// Compile with: -DFUNDAMENTAL_STARTUP_VERBOSE=1
#if FUNDAMENTAL_STARTUP_VERBOSE
    #define STARTUP_TRACE(name) \
        fun_console_write_line("STARTUP: " name);
#else
    #define STARTUP_TRACE(name) ((void)0)
#endif

void fun_startup_run(void)
{
    STARTUP_TRACE("Platform init");
    fun_platform_init();
    
    STARTUP_TRACE("Logging init");
    fun_logging_init();
}
```

---

### 7. __main() Stub Integration

**Decision**: The existing `__main()` stub in `src/startup/startup.c` will call `fun_startup_run()`.

**Rationale**:
- GCC calls `__main()` automatically when using `-nostdlib` / `-ffreestanding`
- No constructor attributes needed
- Already part of the build system
- Zero configuration for users

**Implementation**:
```c
// src/startup/startup.c

void __main(void)
{
    fun_startup_run();
}

void fun_startup_run(void)
{
    // Execute all phases...
}
```

**Note**: `__main()` is called by GCC-generated code before entering `main()`. This is the standard mechanism for runtime initialization in freestanding environments.

---

## Initialization Order

| Phase | Module | Reason for Order |
|-------|--------|------------------|
| 1 | Platform | CPU detection, basic platform info needed by all |
| 2 | Memory | Allocator must be ready before any allocations |
| 3 | Filesystem | Path handling needed for config file location |
| 4 | Config | Loads fun.ini, needed by logging and network |
| 5 | Logging | Uses config for [logging] section |
| 6 | Network | Uses config for [network] rx_buf_size |
| 7+ | Other modules | Can depend on any of the above |

---

## Module Integration Example

**Logging module integration**:

```c
// src/logging/logging.c
#include "fundamental/startup/startup.h"

static int logging_initialized = 0;
static config_values...;

static void logging_init(void)
{
    // Config already loaded - safe to use
    logging_load_config();
    logging_initialized = 1;
}

FUNDAMENTAL_STARTUP_REGISTER(STARTUP_PHASE_LOGGING, logging_init);
```

**Network module integration**:

```c
// src/network/network.c
#include "fundamental/startup/startup.h"

static void network_init(void)
{
    // Config and logging already initialized
    pool_init();
    // Can now load rx_buf_size from config
}

FUNDAMENTAL_STARTUP_REGISTER(STARTUP_PHASE_NETWORK, network_init);
```

---

## Out of Scope

- Runtime module loading (all init at compile time)
- Dynamic registration (no malloc during startup)
- Init order customization by users (fixed phases)
- Rollback on failure (fail fast)

---

## Future Extensions

| Extension | When |
|-----------|------|
| Per-phase timeout detection | If startup hangs become an issue |
| Startup performance profiling | If slow init needs debugging |
| Shutdown/deinit sequence | If cleanup at exit is needed |
| Conditional initialization | If some modules should be optional |
