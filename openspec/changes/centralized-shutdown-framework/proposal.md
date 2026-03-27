## Why

The Fundamental Library handles startup initialization through `__main()` → `fun_startup_run()` but provides no structured shutdown mechanism. Applications that initialize resources through Fundamental modules (file handles, network, memory pools) have no way to ensure those resources are properly released at termination. This creates a gap in production reliability:

- Resource leaks on unhandled signals (SIGTERM, SIGINT, etc.)  
- Unclean termination during normal exit
- Corruption from in-flight writes interrupted suddenly
- Operational confusion when processes exit without closure

## What Changes

- Create centralized shutdown framework in `src/shutdown/shutdown.c`
- Add signal/event handling abstraction for Linux/Windows 
- Implement shutdown callback registry (reverse order from startup)
- Provide three exit paths: normal, abnormal-app-triggered, and external-signal-triggered
- Make `fun_shutdown_run()` idempotent 

## Capabilities

### New Capabilities
- `shutdown-framework`: Reverse-phase shutdown dispatcher with cleanup contracts  
- `signal-handler`: Cross-platform interception of cooperative termination events
- `exit-contracts`: Registry of cleanup callbacks (symmetric to startup init)

### Removed Capabilities  
None - purely additive

### Modified Capabilities
- `startup`: Startup initialization will now set up matching shutdown capability
- `platform`: Platform module provides signal/termination abstraction

## Impact

- New header: `include/fundamental/shutdown/shutdown.h`
- Modules register shutdown functions with cleanup order (reverse-phased)
- Signal handlers installed at startup time
- Existing emergency exit paths (`__builtin_trap()`) remain for unrecoverable errors
- Applications can now implement clean termination via `fun_shutdown_run(SHUTDOWN_NORMAL)`
- Transparent to current users if they don't need clean termination
