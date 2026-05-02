# Demo Build Verification Log

## 2026-05-02 - Initial Demo Creation

### Console Demo ✅
**Location:** `demos/console/`

**Build Command:**
```bash
gcc --std=c17 -Os -I ../../include demo.c \
    ../../src/console/console.c \
    ../../src/string/stringOperations.c \
    ../../arch/console/windows-amd64/console.c \
    ../../arch/memory/windows-amd64/memory.c \
    -o demo.exe
```

**Output:**
```
=== Console Demo ===
Hello from Fundamental Library!
This demonstrates basic console output.
No newline: part1, part2
```

**Dependencies Discovered:**
- `src/console/console.c` - Core console implementation
- `src/string/stringOperations.c` - Required for `fun_string_length()`
- `arch/console/windows-amd64/console.c` - Windows-specific console I/O
- `arch/memory/windows-amd64/memory.c` - Memory allocator

**Issues Encountered:**
1. ❌ Missing `stringOperations.c` - Console uses `fun_string_length()` internally
2. ❌ Non-existent `arch/string/windows-amd64/string.c` - String functions are all in `src/string/`

---

### Logging Demo ✅
**Location:** `demos/logging/`

**Build Command:**
```bash
gcc --std=c17 -Os \
    -I ../../include \
    -D FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_DEBUG \
    -D FUNDAMENTAL_LOG_OUTPUT_CONSOLE=1 \
    -D FUNDAMENTAL_LOG_OUTPUT_FILE=0 \
    demo.c \
    ../../src/logging/logging.c \
    ../../src/console/console.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    ../../src/string/stringValidation.c \
    ../../arch/logging/windows-amd64/logging.c \
    ../../arch/console/windows-amd64/console.c \
    ../../arch/memory/windows-amd64/memory.c \
    -o demo.exe
```

**Output:**
```
2026-05-02T09:14:14.006Z [INFO] demo.c:12 Application started
2026-05-02T09:14:14.006Z [DEBUG] demo.c:15 Debug: value = 42
2026-05-02T09:14:14.006Z [WARN] demo.c:19 Warning: test
2026-05-02T09:14:14.006Z [ERROR] demo.c:22 Error code: 500
2026-05-02T09:14:14.006Z [INFO] demo.c:24 Application finished
```

**Dependencies Discovered:**
- All 4 string source files required (Conversion, Operations, Template, Validation)
- Console module (logging outputs to console)
- Memory allocator

**Issues Encountered:**
1. ❌ Inline array syntax in macro calls - C doesn't allow casting inline arrays in macro arguments
   - **Wrong:** `log_info("msg", (StringTemplateParam[]){...}, 1)`
   - **Right:** Define array first, then pass: `StringTemplateParam p[] = {...}; log_info("msg", p, 1);`

---

## Key Learnings for Documentation

### 1. Module Dependencies Are Not Obvious
- Console → needs stringOperations
- Logging → needs all 4 string files + console
- **Documentation Fix:** Each demo lists exact .c files needed

### 2. Inline Array Syntax Doesn't Work in Macros
- C preprocessor can't handle compound literals in macro arguments
- **Documentation Fix:** Show named array pattern in examples

### 3. LSP Errors Are Misleading
- IDE shows errors for valid code due to missing include paths
- **Documentation Fix:** Tell users to ignore LSP if compilation succeeds

### 4. Build Commands Should Be Copy-Paste Ready
- Don't make users figure out which files to include
- **Documentation Fix:** Include full build commands in comments and .bat files

---

## Test Environment

- **Compiler:** GCC 15.2.0 (MinGW-W64 x86_64-ucrt-posix-seh)
- **Platform:** Windows 10 AMD64
- **Date:** 2026-05-02
