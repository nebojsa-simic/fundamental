# Shutdown Framework Demo

Demonstrates graceful shutdown handling with cleanup on Ctrl+C (SIGINT).

## Features

- Registers cleanup functions that execute on Ctrl+C or SIGTERM
- Handles Windows Ctrl+C, Ctrl+Break, and console close events
- Handles Linux SIGINT and SIGTERM signals
- Writes a file during cleanup to prove handlers executed

## Build

### Windows
```batch
.\build-windows-amd64.bat
```

### Linux
```bash
./build-linux-amd64.sh
```

## Usage

```bash
./demo.exe
```

### What Happens

1. **Normal exit** (wait 10 seconds): Program completes countdown and exits normally. No cleanup file is created.

2. **Graceful shutdown** (press Ctrl+C during countdown):
   - Signal handler catches Ctrl+C
   - Registered cleanup functions execute in reverse phase order
   - `app_cleanup()` writes `shutdown_cleanup.txt`
   - Process exits with code 130 (128 + SIGINT)

### Example Output (Ctrl+C)

```
=== Shutdown Framework Demo ===
Press Ctrl+C during countdown...

10 seconds remaining...
9 seconds remaining...
8 seconds remaining...
^C
=== Cleanup Executed ===
File written: shutdown_cleanup.txt
```

### Verify Cleanup

After pressing Ctrl+C:

```batch
type shutdown_cleanup.txt   # Windows
cat shutdown_cleanup.txt    # Linux
```

Expected content:
```
Shutdown framework demo: cleanup function executed!
```

## How It Works

### 1. Register Cleanup Function

Call `shutdown_register_cleanup()` **before** `fun_startup_run()`:

```c
static void app_cleanup(void)
{
    // Cleanup code here
}

int main(void)
{
    shutdown_register_cleanup(SHUTDOWN_PHASE_APP, app_cleanup);
    fun_startup_run();  // Installs signal handlers
    // ... rest of program
}
```

**Important:** Registration must happen before `fun_startup_run()` because signal handlers are installed during startup.

### 2. Shutdown Phases

Cleanup functions execute in **reverse phase order** (highest to lowest):

| Phase | Constant | Value |
|-------|----------|-------|
| APP | `SHUTDOWN_PHASE_APP` | 99 |
| NETWORK | `SHUTDOWN_PHASE_NETWORK` | 5 |
| CONFIG | `SHUTDOWN_PHASE_CONFIG` | 4 |
| FILESYSTEM | `SHUTDOWN_PHASE_FILESYSTEM` | 3 |
| MEMORY | `SHUTDOWN_PHASE_MEMORY` | 2 |
| PLATFORM | `SHUTDOWN_PHASE_PLATFORM` | 1 |

This ensures resources are cleaned up in reverse order of initialization.

### 3. Signal Handling

**Windows:**
- `SetConsoleCtrlHandler()` catches Ctrl+C, Ctrl+Break, console close
- Calls `fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)`

**Linux:**
- `signal(SIGINT)` catches Ctrl+C
- `signal(SIGTERM)` catches termination requests
- Calls `fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)`

### 4. Shutdown Flow

```
Ctrl+C pressed
    ↓
console_ctrl_handler() or posix_signal_handler()
    ↓
fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)
    ↓
Execute cleanup functions (phase 99 → 1)
    ↓
platform_shutdown_exit(130)
    ↓
Process exits with code 130
```

## API Reference

### Register Cleanup Function

```c
void shutdown_register_cleanup(int phase, void (*handler)(void));
```

- `phase`: Shutdown phase constant (e.g., `SHUTDOWN_PHASE_APP`)
- `handler`: Function to call during shutdown (no parameters, no return)

### Shutdown Types

```c
typedef enum {
    SHUTDOWN_NORMAL = 0,     // Normal program exit
    SHUTDOWN_ABNORMAL = 1,   // Crash or error
    SHUTDOWN_EXTERNAL = 2,   // External signal (Ctrl+C, SIGTERM)
    SHUTDOWN_EMERGENCY = 3,  // System shutdown
} fun_shutdown_type;
```

### Run Shutdown

```c
void fun_shutdown_run(fun_shutdown_type type, int exit_code);
```

- `type`: Shutdown type (determines cleanup behavior)
- `exit_code`: Exit code passed to OS

**Note:** This function does not return - it calls `platform_shutdown_exit()` which terminates the process.

## Dependencies

- Shutdown framework (`src/shutdown/`)
- Signal handlers (`arch/signals/`)
- Console I/O
- File I/O (async write)
- String templates

## Notes

- **Maximum 32 cleanup functions** can be registered
- Cleanup functions should be fast and non-blocking
- Avoid allocating memory in cleanup functions
- File I/O in cleanup should use sync mode for durability
- The `FUNDAMENTAL_SHUTDOWN_REGISTER` macro uses `__attribute__((constructor))` which may not work reliably on all platforms - direct registration via `shutdown_register_cleanup()` is recommended

## See Also

- `include/fundamental/shutdown/shutdown.h` - API header
- `src/shutdown/shutdown.c` - Core implementation
- `arch/signals/*/signal.c` - Platform-specific signal handlers
