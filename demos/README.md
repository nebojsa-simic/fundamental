# Fundamental Library - Demo Applications

## ⚠️ Before Building Any Demo

**Run the validator first:**
```bash
cd demos/<demo-name>
..\validate-demo.bat    # Windows
../validate-demo.sh     # Linux
```

**The validator will BLOCK the build if:**
- Source files referenced don't exist
- Headers referenced don't exist
- Common wrong API patterns detected (e.g., `fun_file_read()`, wrong String usage)
- Build script missing

**If validation fails:**
1. Read the error messages
2. Check `AGENTS.md` section: "Before Writing Demo/Test Code"
3. Read existing test build scripts for correct patterns
4. Re-run validator until it passes

---

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

---

## ⚠️ Before Creating a New Demo - Checklist

**Follow these steps IN ORDER to avoid wasted time:**

- [ ] **1. Read the header** - `cat include/fundamental/<module>/<module>.h`
- [ ] **2. Find existing tests** - `ls tests/ | grep <pattern>`
- [ ] **3. Copy test build script pattern** - `cat tests/<module>/build-windows-amd64.bat`
- [ ] **4. Verify source file locations** - `ls src/<module>/` AND `ls arch/<module>/<platform>/`
- [ ] **5. Write minimal code first** - Just get it compiling
- [ ] **6. Build immediately** - Don't add features until it compiles
- [ ] **7. Test on both platforms** - Windows AND Linux

**Most common mistakes:**
1. ❌ Guessing function names → ✅ Read the header first
2. ❌ Wrong source paths → ✅ Check tests/<module>/build-*.bat
3. ❌ Missing dependencies → ✅ Check what the test includes
4. ❌ Wrong String type → ✅ It's a struct with `.data` and `.size`, not `char*`

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

---

## How to Find the Correct API (Before Writing Code)

**CRITICAL:** Always verify the actual API before writing demo code. Follow this checklist:

### Step 1: Check the Header File

```bash
# Find the header
ls include/fundamental/<module>/

# Read the API
cat include/fundamental/<module>/<module>.h
```

**Example:**
```bash
ls include/fundamental/file/      # Shows: file.h
cat include/fundamental/file/file.h  # Shows: fun_read_file_in_memory(), not fun_file_read()
```

### Step 2: Check Existing Tests for Usage Patterns

```bash
# Find tests for the module
ls tests/ | grep <module>

# Read the test build script
cat tests/<module>/build-windows-amd64.bat

# Read the test code
cat tests/<module>/test.c
```

**Example:**
```bash
ls tests/ | grep file              # Shows: fileRead, fileWrite, fileAppend, fileLock
cat tests/fileRead/build-windows-amd64.bat  # Shows exact source files needed
```

### Step 3: Check Source File Locations

```bash
# Some modules have src/<module>/<module>.c
ls src/<module>/

# Some modules are ONLY in arch/
ls arch/<module>/<platform>/
```

**Common patterns:**
| Module | Source Location | Arch Location |
|--------|-----------------|---------------|
| console | `src/console/console.c` | `arch/console/<platform>/console.c` |
| logging | `src/logging/logging.c` | `arch/logging/<platform>/logging.c` |
| hashmap | `src/hashmap/hashmap.c` | (none) |
| array | `src/array/array.c` | (none) |
| file I/O | (none) | `arch/file/<platform>/fileRead*.c`, `fileWrite*.c` |
| filesystem | `src/filesystem/*.c` | `arch/filesystem/<platform>/*.c` |
| string | `src/string/*.c` (4 files) | (none) |
| memory | (none) | `arch/memory/<platform>/memory.c` |
| async | `src/async/async.c` | `arch/async/<platform>/async.c` |

### Step 4: Check Existing Demo Build Scripts

```bash
# Copy the pattern from existing demos
cat demos/console/build-windows-amd64.bat
cat demos/logging/build-windows-amd64.bat
```

### Step 5: Verify Function Signatures

**DO NOT GUESS function names.** Always check:

```bash
# Search for function in headers
grep -r "fun_" include/fundamental/<module>/

# Or read the header directly
cat include/fundamental/<module>/<module>.h
```

**Common mistakes to avoid:**
- ❌ `fun_file_read()` - ✅ `fun_read_file_in_memory(Read params)`
- ❌ `fun_console_write_line("text")` - ✅ `fun_console_write_line((String){.data="text", .size=len})`
- ❌ `src/file/file.c` - ✅ `arch/file/windows-amd64/fileRead.c` (+ other fileRead*.c files)
- ❌ `fundamental/console/console.h` - ✅ `fundamental/console/console.h` (this one is correct)
- ❌ `arch/filesystem/windows-amd64/file.c` - ✅ `arch/file/windows-amd64/fileRead.c`

### Step 6: Build and Test Immediately

```bash
cd demos/<your-demo>
.\build-windows-amd64.bat
.\demo.exe
```

If it doesn't compile, fix the issues BEFORE proceeding.

---

## Module Dependency Quick Reference

### Console I/O
```
Headers: fundamental/console/console.h
Sources: src/console/console.c, src/string/stringOperations.c
Arch: arch/console/<platform>/console.c, arch/memory/<platform>/memory.c
```

### Logging
```
Headers: fundamental/logging/logging.h
Defines: FUNDAMENTAL_LOG_LEVEL, FUNDAMENTAL_LOG_OUTPUT_CONSOLE, FUNDAMENTAL_LOG_OUTPUT_FILE
Sources: src/logging/logging.c, src/console/console.c, src/string/*.c (all 4 files)
Arch: arch/logging/<platform>/logging.c, arch/console/<platform>/console.c, arch/memory/<platform>/memory.c
```

### File I/O (Read/Write/Append)
```
Headers: fundamental/file/file.h
Types: Read, Write, Append, AsyncResult
Sources: (none in src/)
Arch: arch/file/<platform>/fileRead.c, fileReadMmap.c, fileReadRing.c (for read)
      arch/file/<platform>/fileWrite.c, fileWriteMmap.c, fileWriteRing.c (for write)
      arch/file/<platform>/fileAppend.c (for append)
Also needs: src/async/async.c, arch/async/<platform>/async.c, src/string/*.c
```

### Filesystem (Directory/Path)
```
Headers: fundamental/filesystem/filesystem.h, fundamental/filesystem/path.h
Sources: src/filesystem/directory.c, src/filesystem/path.c, src/filesystem/file_exists.c, 
         src/filesystem/file_size.c, src/filesystem/walk.c, src/tsv/tsv.c
Arch: arch/filesystem/<platform>/*.c
Also needs: arch/memory/<platform>/memory.c, src/string/*.c
```

### Collections (Hashmap, Array, Set, RBTree)
```
Headers: fundamental/hashmap/hashmap.h, fundamental/array/array.h, etc.
Sources: src/hashmap/hashmap.c, src/array/array.c, etc.
Arch: (none, except memory)
Also needs: arch/memory/<platform>/memory.c, src/async/async.c, arch/async/<platform>/async.c
```

### String Operations
```
Headers: fundamental/string/string.h
Sources: src/string/stringConversion.c, src/string/stringOperations.c, 
         src/string/stringTemplate.c, src/string/stringValidation.c
Arch: (none)
```

### Memory
```
Headers: fundamental/memory/memory.h
Sources: (none in src/)
Arch: arch/memory/<platform>/memory.c
```

### Async
```
Headers: fundamental/async/async.h
Sources: src/async/async.c
Arch: arch/async/<platform>/async.c
```

---

## Build Script Template

### Windows (build-windows-amd64.bat)
```batch
@ECHO OFF
REM <Demo Name> - Build Script
REM Dependencies: <list modules>

gcc --std=c17 -Os ^
    -I ../../include ^
    demo.c ^
    ../../src/<module1>/<module1>.c ^
    ../../src/<module2>/<file1>.c ^
    ../../src/<module2>/<file2>.c ^
    ../../arch/<module1>/<platform>/<module1>.c ^
    ../../arch/<module2>/<platform>/<module2>.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -o demo.exe
strip --strip-unneeded demo.exe
```

### Linux (build-linux-amd64.sh)
```bash
#!/bin/bash
# <Demo Name> - Build Script
# Dependencies: <list modules>

gcc --std=c17 -Os \
    -I ../../include \
    demo.c \
    ../../src/<module1>/<module1>.c \
    ../../src/<module2>/<file1>.c \
    ../../arch/<module1>/<platform>/<module1>.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o demo
```
