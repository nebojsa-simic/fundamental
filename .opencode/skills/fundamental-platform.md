---
name: fundamental-platform
description: Platform detection with Fundamental Library - OS and architecture at compile time
license: MIT
compatibility: Complements fundamental-expert skill
metadata:
  author: fundamental-library
  version: "1.0"
  category: platform-detection
  related: fundamental-async, fundamental-console
---

# Fundamental Library - Platform Detection Skill

I provide copy-paste examples for platform detection using the Fundamental Library.

---

## Quick Reference

| Task | Function | Example |
|------|----------|---------|
| Get OS + arch | `fun_platform_get()` | See below |
| OS to string | `fun_platform_os_to_string()` | `"windows"`, `"linux"`, `"darwin"` |
| Arch to string | `fun_platform_arch_to_string()` | `"amd64"`, `"arm64"` |
| Full string | `fun_platform_to_string()` | `"windows-amd64"` |

**OS Values:**
- `PLATFORM_OS_WINDOWS`
- `PLATFORM_OS_LINUX`
- `PLATFORM_OS_DARWIN`
- `PLATFORM_OS_UNKNOWN`

**Arch Values:**
- `PLATFORM_ARCH_AMD64`
- `PLATFORM_ARCH_ARM64`
- `PLATFORM_ARCH_UNKNOWN`

---

## Task: Get Current Platform

Detect OS and architecture at runtime (resolved at compile time via preprocessor macros).

```c
#include "platform/platform.h"

void platform_detect_example(void)
{
    PlatformResult r = fun_platform_get(NULL);
    Platform p = r.value;  // error is always NO_ERROR

    if (p.os == PLATFORM_OS_WINDOWS) {
        // Windows-specific code
    } else if (p.os == PLATFORM_OS_LINUX) {
        // Linux-specific code
    } else if (p.os == PLATFORM_OS_DARWIN) {
        // macOS-specific code
    }

    if (p.arch == PLATFORM_ARCH_AMD64) {
        // x86-64 code
    } else if (p.arch == PLATFORM_ARCH_ARM64) {
        // ARM64 code
    }
}
```

**Key Points:**
- Detection is compile-time via preprocessor — no runtime cost
- `fun_platform_get(NULL)` — pass NULL if you don't need an output pointer
- Error is always `NO_ERROR`; no need to check it

---

## Task: Get Platform Into Output Pointer

Fill a caller-allocated `Platform` struct.

```c
#include "platform/platform.h"

void platform_output_example(void)
{
    Platform platform;
    fun_platform_get(&platform);  // fills platform in place

    // use platform.os, platform.arch
}
```

---

## Task: Convert OS/Arch to String

Get human-readable names for logging or build script selection.

```c
#include "platform/platform.h"
#include "console/console.h"

void platform_names_example(void)
{
    PlatformResult r = fun_platform_get(NULL);
    Platform p = r.value;

    String os_name   = fun_platform_os_to_string(p.os);    // "windows", "linux", "darwin"
    String arch_name = fun_platform_arch_to_string(p.arch); // "amd64", "arm64"

    fun_console_write(os_name);
    fun_console_write("-");
    fun_console_write_line(arch_name);
}
```

---

## Task: Get Full Platform String

Write the combined `"os-arch"` string into a caller-provided buffer.

```c
#include "platform/platform.h"
#include "console/console.h"

void platform_to_string_example(void)
{
    PlatformResult r = fun_platform_get(NULL);
    Platform p = r.value;

    char buf[32];  // must be at least 32 bytes
    voidResult res = fun_platform_to_string(p, buf);

    if (fun_error_is_error(res.error)) {
        // Only fails if buf is NULL
        return;
    }

    fun_console_write_line((String)buf);  // e.g. "windows-amd64"
}
```

**Key Points:**
- Buffer must be at least 32 bytes
- Only error case is NULL buffer
- Format is always `"<os>-<arch>"`

---

## Common Patterns

### Select Script by Platform
```c
PlatformResult r = fun_platform_get(NULL);
Platform p = r.value;

const char *script = (p.os == PLATFORM_OS_WINDOWS)
    ? "build-windows-amd64.bat"
    : "build-linux-amd64.sh";
```

### Branch on Both OS and Arch
```c
Platform p = fun_platform_get(NULL).value;

if (p.os == PLATFORM_OS_WINDOWS && p.arch == PLATFORM_ARCH_AMD64) {
    // windows/amd64 path
} else if (p.os == PLATFORM_OS_LINUX && p.arch == PLATFORM_ARCH_ARM64) {
    // linux/arm64 path
} else {
    // fallback
}
```

### Log Platform at Startup
```c
char buf[32];
fun_platform_to_string(fun_platform_get(NULL).value, buf);
fun_console_write("Platform: ");
fun_console_write_line((String)buf);
```

---

## See Also

- **[fundamental-console.md](fundamental-console.md)** - Writing platform info to output
- **[fundamental-async.md](fundamental-async.md)** - Spawning platform-specific processes
- **[include/platform/platform.h](../../include/platform/platform.h)** - Complete platform API
