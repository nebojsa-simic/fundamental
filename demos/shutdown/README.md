# Shutdown Framework Demo

Demonstrates the Fundamental Library shutdown framework with graceful cleanup on Ctrl+C.

## Features

- Registers cleanup function using `FUNDAMENTAL_SHUTDOWN_REGISTER()`
- Handles Ctrl+C (Windows) and SIGINT/SIGTERM (Linux)
- Writes a file during cleanup to prove shutdown handlers ran
- 10-second countdown with string template formatting

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

```batch
demo.exe
```

### What Happens

1. **Normal exit** (wait 10 seconds): Program completes countdown and exits. No cleanup file is written.

2. **Graceful shutdown** (press Ctrl+C during countdown):
   - Shutdown framework catches the signal
   - Registered cleanup function runs
   - Writes `shutdown_cleanup.txt` with proof message
   - Exits cleanly

### Example Output (Ctrl+C)

```
=== Shutdown Framework Demo ===

This demo waits 10 seconds.
Press Ctrl+C to trigger graceful shutdown.

Starting countdown...

10 seconds remaining...
9 seconds remaining...
^C
=== Cleanup Executed ===
File written: shutdown_cleanup.txt
This proves shutdown handlers ran!
```

### Verify Cleanup

After pressing Ctrl+C, check for the file:

```batch
type shutdown_cleanup.txt   # Windows
cat shutdown_cleanup.txt    # Linux
```

Expected content:
```
Shutdown framework demo: cleanup function executed!
```

## How It Works

1. **Registration**: `app_cleanup()` is registered at `SHUTDOWN_PHASE_APP` using the `FUNDAMENTAL_SHUTDOWN_REGISTER` macro

2. **Signal Handling**: 
   - Windows: `SetConsoleCtrlHandler()` catches Ctrl+C
   - Linux: `signal(SIGINT)` catches Ctrl+C

3. **Shutdown Flow**:
   - Signal received → `fun_shutdown_run(SHUTDOWN_EXTERNAL, 130)`
   - Executes cleanup functions in reverse phase order
   - `app_cleanup()` writes file
   - Process exits with code 130

## Dependencies

- Shutdown framework (`src/shutdown/`)
- Console I/O
- File I/O (async write)
- String operations and templates
- Signal handlers (`arch/signals/`)
