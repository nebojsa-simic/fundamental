## Why

Multiple library modules need initialization at startup (logging config loading, network pool setup, etc.). Currently each module would use its own GCC constructor, leading to:

- Uncontrolled initialization order (what if logging init runs before config is ready?)
- No visibility into what runs at startup
- Difficult to debug startup issues
- No central place to add future initialization

## What Changes

- Create centralized startup initialization framework in `src/startup/`
- Replace `__main()` stub with dispatcher that calls all module inits
- Controlled, documented initialization order
- Optional startup logging for debugging init sequence

## Capabilities

### New Capabilities
- `startup-framework`: Central initialization dispatcher with ordered phases
- `startup-logging`: Optional logging of initialization sequence (once logging is ready)

### Modified Capabilities
- `startup`: Existing `__main()` stub will call `fun_startup_run()` dispatcher

## Impact

- New header: `include/fundamental/startup/startup.h`
- Modules register init functions via `FUNDAMENTAL_STARTUP_REGISTER(priority, function)`
- Initialization order: platform → memory → filesystem → config → logging → network → others
- Transparent to library users — still zero-effort initialization
