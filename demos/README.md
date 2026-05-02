# Fundamental Library - Demo Applications

This directory contains working demo applications that demonstrate each module of the Fundamental Library. **All demos are guaranteed to compile and run** - they are tested alongside the library.

## Quick Start

### Windows (MinGW)
```batch
cd demos\logging
.\build-windows-amd64.bat
.\demo.exe
```

### Linux
```bash
cd demos/logging
./build-linux-amd64.sh
./demo
```

## Available Demos

| Demo | Modules Demonstrated | Complexity | Build & Run |
|------|---------------------|------------|-------------|
| **console** | Console I/O | Beginner | `cd demos/console && .\build-windows-amd64.bat && .\demo.exe` |
| **logging** | Logging, String templates | Beginner | `cd demos/logging && .\build-windows-amd64.bat && .\demo.exe` |

## Documentation Conventions

### Build Dependency Lists

Each demo includes a **Build Dependencies** section that lists exactly which source files are needed:

```
Core: src/logging/logging.c
String: src/string/stringConversion.c, src/string/stringOperations.c, 
        src/string/stringTemplate.c, src/string/stringValidation.c
Arch: arch/logging/<platform>/logging.c, 
      arch/console/<platform>/console.c,
      arch/memory/<platform>/memory.c
```

**Why this matters:** The Fundamental Library is modular - you only compile what you use. This is different from monolithic libraries where you link a single .lib or .a file.

### Compile-Time Configuration

Many modules use compile-time configuration via `#define` macros:

```c
#define FUNDAMENTAL_LOG_LEVEL LOG_LEVEL_DEBUG
#define FUNDAMENTAL_LOG_OUTPUT_CONSOLE 1
#define FUNDAMENTAL_LOG_OUTPUT_FILE 0

#include "fundamental/logging/logging.h"
```

**Important:** These defines MUST come before the `#include` statement.

### Entry Point

The library supports two entry point conventions:

1. **Standard `main()`** - Use for most applications
2. **`cli_main()`** - Use when including `arch/startup/*/windows.c` for -nostdlib builds

**Recommendation:** Use standard `main()` unless you need zero-stdlib binaries.

## Common Build Patterns

### Minimal Console Application

**Files needed:**
- Core: `src/console/console.c`
- String: `src/string/stringOperations.c`
- Arch: `arch/console/<platform>/console.c`, `arch/memory/<platform>/memory.c`

**Compile:**
```bash
gcc -std=c17 -Os -I include \
    demo.c \
    src/console/console.c \
    src/string/stringOperations.c \
    arch/console/linux-amd64/console.c \
    arch/memory/linux-amd64/memory.c
```

### With Logging

**Files needed:**
- Logging: `src/logging/logging.c`
- Console: `src/console/console.c`
- String: `src/string/stringConversion.c`, `src/string/stringOperations.c`, `src/string/stringTemplate.c`, `src/string/stringValidation.c`
- Arch: `arch/logging/<platform>/logging.c`, `arch/console/<platform>/console.c`, `arch/memory/<platform>/memory.c`

**Compile:**
```bash
gcc -std=c17 -Os -I include \
    -D FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_DEBUG \
    -D FUNDAMENTAL_LOG_OUTPUT_CONSOLE=1 \
    demo.c \
    src/logging/logging.c \
    src/console/console.c \
    src/string/stringConversion.c \
    src/string/stringOperations.c \
    src/string/stringTemplate.c \
    src/string/stringValidation.c \
    arch/logging/linux-amd64/logging.c \
    arch/console/linux-amd64/console.c \
    arch/memory/linux-amd64/memory.c
```

### With Collections

**Files needed:**
- Hashmap: `src/hashmap/hashmap.c`
- Array: `src/array/array.c`
- Arch: `arch/memory/<platform>/memory.c`

**Compile:**
```bash
gcc -std=c17 -Os -I include \
    demo.c \
    src/hashmap/hashmap.c \
    src/array/array.c \
    arch/memory/linux-amd64/memory.c
```

## Verified Test Status

These demos are **tested and verified** to compile and run:

- ✅ **console** - Tested on Windows AMD64 (MinGW 15.2.0)
- ✅ **logging** - Tested on Windows AMD64 (MinGW 15.2.0)

More demos coming soon.

## Error Handling Patterns

All Fundamental Library functions return result types:

```c
#include "fundamental/memory/memory.h"

MemoryResult result = fun_memory_allocate(1024);
if (fun_error_is_ok(result.error)) {
    Memory buffer = result.value;
    // Use buffer...
    fun_memory_free(&buffer);
} else {
    // Handle error
    printf("Error %d: %s\n", result.error.code, result.error.message);
}
```

**Key functions:**
- `fun_error_is_ok(error)` - Check if operation succeeded
- `fun_error_is_error(error)` - Check if operation failed
- `result.value` - The actual return value (on success)
- `result.error` - Error information (code + message)

## String Template Syntax

The logging system uses template parameters:

| Prefix | Type | Example |
|--------|------|---------|
| `${name}` | String | `"Hello ${user}"` |
| `#{name}` | Integer | `"Count: #{count}"` |
| `%{name}` | Double | `"Score: %{score}%"` |
| `*{name}` | Pointer | `"Address: *{ptr}"` |

```c
StringTemplateParam params[] = {
    { "user", { .stringValue = "Alice" } },
    { "count", { .intValue = 42 } },
    { "score", { .doubleValue = 95.7 } }
};
log_info("User ${user} scored #{count} points (%{score}%)", params, 3);
// Output: "User Alice scored 42 points (95.700%)"
```

## Troubleshooting

### "undefined reference to `fun_string_length`"

**Cause:** Console module depends on string functions.

**Solution:** Add `src/string/stringOperations.c` to your build.

### "undefined reference" to other functions

**Cause:** Missing source files in build command.

**Solution:** Check the Build Dependencies section for the module you're using. The Fundamental Library is modular - you must compile each dependency explicitly.

### "macro passed N arguments, but takes just 3"

**Cause:** Inline array syntax doesn't work with logging macros.

**Wrong:**
```c
log_info("Value: #{val}", (StringTemplateParam[]){{"val", {.intValue = 1}}}, 1);
```

**Right:**
```c
StringTemplateParam params[] = { {"val", {.intValue = 1}} };
log_info("Value: #{val}", params, 1);
```

### "file not found" for headers

**Cause:** Wrong include path.

**Solution:** Use `-I ../../include` (adjust based on your directory structure)

### LSP/IDE errors but compiles fine

**Cause:** IDE doesn't understand the include path or compile-time defines.

**Solution:** Ignore LSP errors if compilation succeeds. Configure your IDE's C/C++ extension with:
- Include path: `../../include`
- Preprocessor defines: `FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_DEBUG`, etc.

### Multiple definition of `main`

**Cause:** Including `arch/startup/*/windows.c` which provides its own `main()`.

**Solution:** Use standard MinGW runtime (don't include startup arch files) unless you need `-nostdlib` builds.

## Next Steps

After running the demos:

1. **Copy a demo** - Start with the demo closest to your use case
2. **Modify incrementally** - Change one thing at a time
3. **Check the headers** - `include/fundamental/*/` has full API documentation
4. **Read the tests** - `tests/*/` shows all edge cases and error handling

## Contributing

When adding new demos:

1. Keep them minimal and focused
2. Include build scripts for both Windows and Linux
3. Add to the table above
4. Ensure they compile in CI
