## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                   SHUTDOWN FRAMWORK ARCHITECTURE                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  STARTUP PHASE (symmetry)                                       │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │  __main() → fun_startup_run()                              │ │
│  │  Phase 1: Platform → Phase 2: Memory → Phase 3: File...    │ │
│  │                                    ↓                       │ │
│  │  Each phase registers corresponding shutdown callback      │ │
│  └────────────────────────────────────────────────────────────┘ │
│                    │                                            │
│                    ▼ (automatic)                                │
│  SHUTDOWN PHASE (inverse symmetry)                              │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │  fun_shutdown_run(TYPE)                                    │ │
│  │  Phase N: File → Phase N-1: Memory → Phase N-2: Platform   │ │
│  │  (cleanup callbacks execute in reverse order)              │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                 │
│  SIGNAL PATH                                                    │
│  ┌─────────────────┐     ┌─────────────────┐    ┌─────────────┐ │
│  │                 │───▶│                 │───▶│             │ │
│  │  Linux:         │    │ fun_signal_     │    │  Call       │ │
│  │  sigaction()    │    │  handler()      │    │  fun_shutdown_run() │ │
│  │  Windows:       │    │                 │    │             │ │
│  │  SetConsoleCtrlHandler()  │    │ (cleanup)    │ │
│  └─────────────────┘    └─────────────────┘    └─────────────┘ │
│                                                                 │
│  THREE EXIT PATHS                                               │
│  ┌─────────────────┬─────────────────┬─────────────────────────┐ │
│  │   CASE 1        │    CASE 2       │     CASE 3            │ │
│  │   Normal Exit   │   App-Triggered │   External Signal     │ │
│  │   (return 0)    │   (error code)  │   (SIGTERM, etc.)     │ │
│  │                 │   fun_shutdown_run(ABNORMAL_APP)         │ │
│  │                 │   │              │   fun_shutdown_run(EXTERNAL) │ │
│  │                 │   ▼              │   │                   │ │
│  │  fun_shutdown_run(NORMAL)          │   │                   │ │
│  │                 │                  │   ▼                   │ │
│  │                 │                  │  (signal handler)     │ │
│  └─────────────────┴──────────────────┴─────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Design Decisions

### 1. Symmetric Startup/Shutdown Contract

**Decision**: Shutdown phases mirror startup in reverse order (N, N-1, N-2, ..., 1).

**Rationale**:
- Resources that were created last are released first (stack-like)
- Network module inits last, cleans up first (no connections during file cleanup)
- Memory pool allocates early, frees late (no dangling ptr during cleanup)
- Symmetry makes reasoning easier (what goes up must come down)

**Implementation**:
```c
// startup.c
static const startup_phase phases_init[] = {
    {PHASE_PLATFORM, platform_init},  
    {PHASE_MEMORY, memory_init},
    {PHASE_FILE, file_init},
    {PHASE_CONFIG, config_init},
    {PHASE_NETWORK, network_init}
};

// Corresponding shutdown order: network → config → etc.
static const shutdown_phase phases_cleanup[] = {
    {PHASE_NETWORK, network_deinit},
    {PHASE_CONFIG, config_deinit},
    {PHASE_FILE, file_deinit},  
    {PHASE_MEMORY, memory_deinit},
    {PHASE_PLATFORM, platform_deinit}
};
```

---

### 2. Idempotent Shutdown Function

**Decision**: `fun_shutdown_run()` can be called multiple times without side effects.

**Rationale**:
- Signal handlers don't need to check if already running
- App-triggered cleanup won't crash if external signal interrupts
- Emergency path can call same function without duplication
- Race condition protection during async shutdown

**Implementation**:
```c
static _Atomic int shutdown_started = 0;
static _Atomic int shutdown_completed = 0;

void fun_shutdown_run(shutdown_type type, int exit_code)
{
    int expected = 0;
    if (!atomic_cmpxchg(&shutdown_started, &expected, 1)) {
        // Another thread is handling shutdown - return gracefully
        return;
    }
    
    // Execute phased cleanup...
    execute_shutdown_phases(type, exit_code);

    shutdown_completed = 1;
}
```

---

### 3. Signal/Event Abstraction Layer

**Decision**: Abstract platform specific signaling behind one API.

**Rationale**:
- Linux: Signals (SIGTERM, SIGINT, SIGHUP, etc.)
- Windows: Console events (CTRL_C, CTRL_CLOSE)
- Service modes: Service control handlers
- Unified API hides complexity

**Linux Implementation**:
```c
// arch/signals/linux-amd64/signal.c
static void signal_handler(int sig, siginfo_t *info, void *context)
{
    fun_signal_event event = convert_linux_signal(sig);
    fun_signal_dispatch(event);
}

// Subscribe to all graceful termination vectors  
void linux_signals_install(void)
{
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    
    sigaction(SIGTERM, &sa, NULL);  // Graceful termination
    sigaction(SIGINT, &sa, NULL);   // Ctrl+C  
    sigaction(SIGQUIT, &sa, NULL);  // Quit with dump
    sigaction(SIGHUP, &sa, NULL);   // Hangup/reload
}

// Signal dispatch examples:
void fun_signal_dispatch(fun_signal_event event)
{
    switch (event.type) {
    case EVENT_GRACEFUL_TERM:
        fun_shutdown_run(SHUTDOWN_EXTERNAL, 0);  // Exit code 0 for graceful
        break;
    case EVENT_INTERRUPT:
        fun_shutdown_run(SHUTDOWN_EXTERNAL, 130);  // Exit 130 for Ctrl+C
        break;
    case EVENT_FORCE_QUIT:
        fun_shutdown_run(SHUTDOWN_EXTERNAL, 131); // Exit 131 for SIGQUIT
        break;
    }
}
```

**Windows Implementation**:
```c
// arch/signals/windows-amd64/signal.c
BOOL WINAPI windows_ctrl_handler(DWORD ctrl_type)
{
    switch (ctrl_type) {
    case CTRL_C_EVENT:      // Ctrl+C
        fun_shutdown_run(SHUTDOWN_EXTERNAL, 130);
        return TRUE;  // Handled - prevent default behavior
    case CTRL_CLOSE_EVENT:  // Console closing
        fun_shutdown_run(SHUTDOWN_EXTERNAL, 0); 
        Sleep(2000);  // Allow time for cleanup
        return TRUE;  // Handled - prevent premature termination
    }
}

void windows_signals_install(void)
{
    SetConsoleCtrlHandler(windows_ctrl_handler, TRUE);
}
```

**Windows Implementation**:
```c
// arch/signals/windows-amd64/signal.c
BOOL WINAPI windows_ctrl_handler(DWORD ctrl_type)
{
    fun_signal_event event = convert_windows_event(ctrl_type);
    return fun_signal_dispatch(event);
}

void windows_signals_install(void)
{
    SetConsoleCtrlHandler(windows_ctrl_handler, TRUE);
}
```

---

### 4. Three-Tier Termination Strategy

**Decision**: Different exit paths for different scenarios.

**Normal Exit**:
- Application completes normally (returns from main)
- `fun_shutdown_run(SHUTDOWN_NORMAL)` 
- Everything cleaned up, orderly termination

**Abnormal App-Generated**:
- Application encounters fatal error
- `fun_shutdown_run(SHUTDOWN_ABNORMAL)` 
- Try cleanup for post-mortem, then exit

**External Signal**:
- Operating system requests termination
- Signal handler calls `fun_shutdown_run(SHUTDOWN_EXTERNAL)`  
- Quick cleanup, no time-consuming operations

**Implementation**:
```c
typedef enum {
    SHUTDOWN_NORMAL,
    SHUTDOWN_ABNORMAL, 
    SHUTDOWN_EXTERNAL,
    SHUTDOWN_EMERGENCY  // For unrecoverable system problems
} fun_shutdown_type;

void fun_shutdown_run(fun_shutdown_type type, int exit_code);
```

**Pattern**:
```c
// App-triggered cleanup  
if (app_error_condition) {
    fun_shutdown_run(SHUTDOWN_ABNORMAL, 1);  // With exit code
    return;
}
```

---

### 5. Registry-based Cleanup Contract

**Decision**: Modules register cleanup functions at startup time.

**Rationale**:
- Startup handles both initialization AND cleanup registration
- No duplication of module-awareness
- Modules cleanup in same order as initialization was handled

**Implementation**:
```c
// In module source code (same as startup registration)
static void network_module_init(void) { /* ... */ }
static void network_module_deinit(void) { /* ... */ }

// Register both at startup time
void __attribute__((constructor)) network_register(void)
{
    FUNDAMENTAL_STARTUP_REGISTER(PHASE_NETWORK, network_module_init);
    FUNDAMENTAL_SHUTDOWN_REGISTER(PHASE_NETWORK, network_module_deinit);  // New!
}
```

**Startup Framework Handles Both**:
```c
// At startup
void fun_startup_run(void)
{
    for (int i = 0; i < NUM_PHASES; i++) {
        execute_phase_init(i);
        register_corresponding_deinit(i);  // Store for shutdown
    }
}
```

---

### 6. No Emergency Override

**Decision**: Even signal handlers call cleanup, but with time bounds.

**Rationale**:
- SIGKILL still cannot be caught (OS limits)
- SIGTERM/SIGINT get cleanup attempt 
- Still can specify "fast cleanup" mode for signals

**Tradeoff**: Signal handlers briefly delay process death to allow cleanup.

---

### 7. App-Initiated Shutdown Is Complete

**Decision**: `fun_shutdown_run()` handles complete shutdown - no further `exit()` call needed.

**Rationale**:
- Applications just call `fun_shutdown_run(SHUTDOWN_ABNORMAL)`
- Framework handles all cleanup activities fully
- App naturally returns to main() context after, eliminating need for extra steps
- Simpler for application writers (one call completes shutdown)

**Pattern**:
```c
// App-triggered cleanup
if (app_error_condition) {
    fun_shutdown_run(SHUTDOWN_ABNORMAL);
    return;  // App returns to main() which handles actual exit
}
```

---

### 10. Phase Definition Consistency

**Decision**: Shutdown phases mirror startup phases, including app-level phases at 99.

**Rationale**:
- Symmetric with startup registration system
- Applications initialize at STARTUP_PHASE_APP=99, shut down at SHUTDOWN_PHASE_APP=99  
- Allows application logic to access infrastructure during termination
- Consistent mental model (what goes up, comes down)

**Implementation**:
```c
// Consistent with startup.h
#define SHUTDOWN_PHASE_PLATFORM    1
#define SHUTDOWN_PHASE_MEMORY      2  
#define SHUTDOWN_PHASE_FILESYSTEM  3
#define SHUTDOWN_PHASE_CONFIG      4
#define SHUTDOWN_PHASE_LOGGING     5
#define SHUTDOWN_PHASE_NETWORK     6
#define SHUTDOWN_PHASE_APP        99  // Matches STARTUP_PHASE_APP for symmetry
```

This enables the registration mapping:
```c
// At startup:
FUNDAMENTAL_STARTUP_REGISTER(STARTUP_PHASE_APP, app_initialize);

// At shutdown:
FUNDAMENTAL_SHUTDOWN_REGISTER(SHUTDOWN_PHASE_APP, app_deinit);
```

---

### 11. Emergency System Termination

**Decision**: Use `__builtin_trap()` for unrecoverable system integrity violations only.

**Rationale**:
- Not all errors warrant full cleanup (memory corruption, invariant failures)
- Emergency halt preserves system state for debugging
- Distinction: application errors (graceful shutdown) vs system integrity violations
Startup Sequence: 
FUNDAMENTAL -> APP (phase 99)  
(App sees FUNDAMENTAL available)

Shutdown Sequence (reverse):
APP (phase 99) -> FUNDAMENTAL
(App has access to FUNDAMENTAL during cleanup)
```

**Registration Interface**:
```c
// App provides cleanup function
void myapp_cleanup(void) {
    // Drain application queues  
    // Flush application buffers
    // Network still available to notify clients
    // Files still available to save state
}

// Applications register at APP phase (matching init)
FUNDAMENTAL_SHUTDOWN_REGISTER(SHUTDOWN_PHASE_APP, myapp_cleanup);
```

**Execution Order**:
```c
// During signal handling:
fun_shutdown_run(TYPE, exit_code);

// Executes:
execute_shutdown_phase(SHUTDOWN_PHASE_APP);    // App cleanup (includes callbacks)
execute_shutdown_phase(NETWORK_PHASE);         // Then network teardown
execute_shutdown_phase(etc...);                // All other infrastructure
```

## Integration Patterns

### For Startup Framework:
```c
// Enhance existing startup to also register shutdowns
typedef struct {
    int phase;
    void (*init_fn)(void);
    void (*deinit_fn)(void);  // New
} init_deinit_pair;

// Build both init and deinit arrays
```

### For Module Writers:
```c
// Current registration
FUNDAMENTAL_STARTUP_REGISTER(PHASE_NETWORK, my_module_init);

// New registration pattern  
FUNDAMENTAL_SHUTDOWN_REGISTER(PHASE_NETWORK, my_module_deinit);
```

### For Applications:
```c
// Normal return path from main() triggers fast cleanup
// Signal handler triggers quick cleanup  
// Exception path triggers diagnostic cleanup
```

---

## Out of Scope

- Windows service control handlers (separate concern)
- Linux daemonization patterns (external to framework) 
- Core dump generation (OS responsibility)
- Debugger attachment detection (not in scope)

---

## Future Extensions

| Extension | When |
|-----------|------|
| Timeout cleanup | If cleanup phases stall |
| Diagnostic dumps | During abnormal shutdown |
| Hook extensibility | If user plugins needed |  
| Process restart | If service-style restart needed |
