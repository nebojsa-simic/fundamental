# Fundamental Library

A simple replacement for the C standard library.

## Overview

The Fundamental Library is a complete reimplementation of standard C library functionality without dependencies on the standard C library. It follows a kernel-style architecture with platform-specific optimizations and explicit error handling throughout.

## Quick Start

### **Working Demos Available**

See [`demos/`](demos/) for **tested, working examples**:

```bash
cd demos/console
.\build-windows-amd64.bat
.\demo.exe

cd demos/logging
.\build-windows-amd64.bat
.\demo.exe
```

Each demo includes:
- Complete source code that compiles on first try
- Build scripts for Windows and Linux
- Documented dependencies

### Basic String Operations

```c
#include "string/string.h"
#include "memory/memory.h"

// Caller allocates output buffer
char output[256];
String source = "Hello, World!";

// Copy string
fun_string_copy(source, output);

// Get length
StringLength len = fun_string_length(source);

// Convert integer to string
fun_string_from_int(42, 10, output);
```

### Memory Management

```c
#include "memory/memory.h"

// Allocate memory
MemoryResult result = fun_memory_allocate(1024);
if (fun_error_is_error(result.error)) {
    // Handle allocation failure
    return;
}

Memory buffer = result.value;

// Use buffer...

// Free memory (caller responsibility)
voidResult free_result = fun_memory_free(&buffer);
```

### String Templates

```c
#include "string/string.h"

char output[512];

// Template prefixes: ${string} #{int} %{double} *{pointer}
String template = "Hello ${name}, you have #{age} messages";

StringTemplateParam params[] = {
    { "name", { .stringValue = "Alice" } },
    { "age", { .intValue = 42 } }
};

fun_string_template(template, params, 2, output);
// Result: "Hello Alice, you have 42 messages"
```

### Async File Operations

```c
#include "file/file.h"

char buffer[2048];
Memory output = { .ptr = buffer, .size = sizeof(buffer) };

Read read_params = {
    .file_path = "/path/to/file.txt",
    .output = output,
    .bytes_to_read = 2048,
    .mode = FILE_MODE_AUTO
};

AsyncResult read_result = fun_file_read(&read_params);
fun_async_await(&read_result, 5000);  // Wait up to 5 seconds

if (read_result.status == ASYNC_COMPLETED) {
    // File contents in buffer
}
```

### Logging

```c
#define FUNDAMENTAL_LOG_LEVEL LOG_LEVEL_DEBUG
#define FUNDAMENTAL_LOG_OUTPUT_CONSOLE 1

#include "logging/logging.h"

log_info("Application started", NULL, 0);

StringTemplateParam params[] = {
    { "user", { .stringValue = "Alice" } },
    { "count", { .intValue = 42 } }
};
log_info("User ${user} processed #{count} items", params, 2);

log_error("Failed to connect to ${host}",
    (StringTemplateParam[]){{"host", {.stringValue = "db.example.com"}}}, 1);
```

### Console Output

```c
#include "console/console.h"

fun_console_write_line("Hello, World!");
fun_console_write("No newline...");
fun_console_write("...combined\n");
fun_console_flush();
```

### Networking - TCP Client

```c
#include "network/network.h"

// Parse address
NetworkAddressResult addr_result = fun_network_address_parse("127.0.0.1:8080");
if (fun_error_is_ok(addr_result.error)) {
    // Connect
    TcpNetworkConnection conn;
    AsyncResult result = fun_network_tcp_connect(addr_result.value, &conn);
    fun_async_await(&result, 5000);
    
    // Send
    const char *msg = "Hello";
    fun_network_tcp_send(conn, msg, 5);
    
    // Receive
    char buf[100];
    NetworkBuffer response = { .data = buf, .length = 0 };
    fun_network_tcp_receive_exact(conn, &response, 100);
    
    fun_network_tcp_close(conn);
}
```

---

## Design Principles

- **No C standard library dependencies** - Complete standalone implementation
- **Caller-allocated memory** - All memory allocation responsibility belongs to the caller
- **Explicit error handling** - All functions return result types with comprehensive error information
- **Descriptive naming** - Function names use `fun_` prefix with full descriptive names (no abbreviations)
- **Linux kernel patterns** - Architecture follows proven kernel design patterns when in doubt
- **Platform-specific optimizations** - Architecture-specific implementations in `arch/` directory

## Project Structure

```
fundamental/
├── arch/                       # Platform-specific implementations
│   ├── async/                 # Async operations (linux-amd64, windows-amd64)
│   ├── config/                # Config env var access per platform
│   ├── console/               # Console I/O per platform
│   ├── file/                  # File I/O implementations per platform
│   ├── filesystem/            # Filesystem operations per platform
│   ├── logging/               # Logging implementations per platform
│   ├── memory/                # Memory management per architecture
│   ├── shutdown/              # Shutdown signal handling per platform
│   ├── startup/               # Platform startup/entry point
│   └── stream/                # Stream I/O per platform
├── include/                   # Public API headers
│   ├── array/                 # Dynamic array interface
│   ├── async/                 # Async operation primitives
│   ├── collections/           # Collections utilities (hash, equality)
│   ├── config/                # Configuration management interface
│   ├── console/               # Console I/O interface
│   ├── error/                 # Error handling system
│   ├── file/                  # File I/O interface
│   ├── filesystem/            # Filesystem operations interface
│   ├── hashmap/               # Hash map interface
│   ├── logging/               # Logging system interface
│   ├── memory/                # Memory management interface
│   ├── network/               # Networking (TCP/UDP) interface
│   ├── rbtree/                # Red-black tree interface
│   ├── set/                   # Set data structure interface
│   ├── shutdown/              # Shutdown framework interface
│   ├── startup/               # Startup framework interface
│   ├── stream/                # Stream I/O interface
│   └── string/                # String operations interface
├── src/                       # Core implementations
│   ├── array/                 # Dynamic array implementation
│   ├── async/                 # Async scheduler and process spawn
│   ├── config/                # Config core, INI parser, CLI parser
│   ├── console/               # Console output implementation
│   ├── filesystem/            # Path and directory operations
│   ├── hashmap/               # Hash map implementation
│   ├── logging/               # Logging system implementation
│   ├── network/               # Network TCP/UDP implementation
│   ├── platform/              # Platform detection and info
│   ├── rbtree/                # Red-black tree implementation
│   ├── set/                   # Set implementation
│   ├── shutdown/              # Shutdown framework implementation
│   ├── startup/               # Startup framework implementation
│   ├── stream/                # Stream lifecycle and file operations
│   ├── string/                # String operations (conversion, templating, validation)
│   └── tsv/                   # TSV parsing and handling
└── tests/                     # Unit tests for all modules
    ├── async/                 # Async and process spawn tests
    ├── collections/           # Array tests
    ├── config/                # Configuration module tests
    ├── console/               # Console output tests
    ├── filesystem/            # Directory and path tests
    ├── hashmap/               # Hash map tests
    ├── memory/                # Memory allocation tests
    ├── process_spawn/         # Process execution tests
    ├── rbtree/                # Red-black tree tests
    ├── set/                   # Set operation tests
    ├── stream/                # Stream I/O tests
    ├── string*/               # String operation tests
    └── file*/                 # File I/O tests (read, write, append, lock)
```

## Features

### **Error Handling System**
- Comprehensive result types for all operations
- Explicit error propagation without exceptions
- Standardized error codes and messages
- `DEFINE_RESULT_TYPE()` macro for custom result types

### **Memory Management**
- Direct syscall-based allocation (Linux)
- Platform-specific optimized implementations
- Caller-controlled allocation patterns
- Operations: allocate, reallocate, free, fill, copy, size query

### **String Operations**
- Complete string manipulation suite
- Template system with type-safe parameter substitution
- Conversion utilities (int, double, pointer to string)
- In-place operations (trim, reverse)
- Out-of-place operations (join, copy)
- Validation and comparison functions

### **Async File I/O**
- Multiple I/O strategies: standard, memory-mapped, ring-based, direct
- Platform-optimized implementations
- Non-blocking read, write, append operations
- File locking for exclusive access with configurable timeout
- File change notifications (platform-specific)
- **Durability modes**: `FILE_DURABILITY_ASYNC` (default), `FILE_DURABILITY_SYNC` (msync/fsync), `FILE_DURABILITY_FULL` (fsync data+metadata)
- **Robustness**: integer overflow protection on all size/offset calculations, runtime page-size detection, proper io_uring CQE consumption with partial-transfer handling

### **Stream Module**
- Asynchronous buffered file I/O
- Read, write, and append modes
- Flow control with `can_read()` and `can_write()` checks
- Position tracking and end-of-stream detection
- Caller-allocated buffers with automatic management

### **Filesystem Operations**
- Directory creation (recursive with parent creation)
- Directory removal and listing
- Path utilities: join, normalize, get parent, get filename
- Platform-independent path separator handling

### **Console I/O**
- Line-buffered stdout output (512-byte buffer)
- Unbuffered stderr output
- Explicit flush control
- Automatic newline handling

### **Collections Module**
- **Dynamic Arrays**: Type-safe vectors with automatic growth, capacity management
- **Hash Maps**: Associative key-value storage with configurable hash functions
- **Red-Black Trees**: Self-balancing ordered trees with O(log n) operations
- **Sets**: Unique-element containers with fast membership checks
- Generic hash functions for primitive types and pointers
- Memory-safe operations with comprehensive error handling

### **Async Process Spawn**
- Asynchronous process execution
- Stdout/stderr capture with configurable buffer sizes
- Process termination and exit code retrieval
- Environment variable control
- Non-blocking wait with `fun_async_await()`

### **Architecture Support**
- Linux AMD64 (implemented)
- Windows AMD64 (implemented)
- Darwin ARM64 (planned)
- Linux ARM64 (planned)
- Extensible architecture system with platform abstractions in `arch/*/`

### **Configuration Management**
- Cascading configuration from CLI arguments, environment variables, and INI files
- Priority cascade: `--config:key=value` → `APPNAME_KEY` env vars → `{app}.ini` file
- Type-safe accessors: `fun_config_get_string()`, `fun_config_get_int()`, `fun_config_get_bool()`
- Ergonomic `get_or_default()` variants for optional configuration values
- `fun_config_has()` for existence checks without type conversion
- INI file auto-located at `{executable_dir}/{app_name}.ini`
- Environment variables transformed: `"database.host"` → `MYAPP_DATABASE_HOST`
- Cross-platform: Linux (`getenv`/`readlink`) and Windows (`GetEnvironmentVariableA`/`GetModuleFileNameA`)

### **Logging System**
- Centralized logging with configurable log levels (TRACE, DEBUG, INFO, WARN, ERROR)
- Template-based log messages with type-safe parameter substitution
- Multiple output targets: console and file (configurable at compile-time and runtime)
- Automatic timestamps, source file, and line number in log output
- Zero-overhead when disabled: compile-time level filtering compiles to `((void)0)`
- Runtime configuration via `fun.ini` `[logging]` section
- Circular dependency prevention: config module cannot call logging functions

### **Startup Framework**
- Ordered initialization phases for library modules
- Phases: Platform → Memory → Filesystem → Config → Logging → Network → Other
- Fail-fast error handling with verbose tracing option (`-DFUNDAMENTAL_STARTUP_VERBOSE=1`)
- Automatic execution via `__main()` at program startup
- Modules register init functions via `FUNDAMENTAL_STARTUP_REGISTER(phase, function)`
- Prevents circular dependencies through enforced phase ordering

### **Shutdown Framework**
- Structured shutdown coordination with cleanup functions
- Executes cleanup in reverse initialization order
- Shutdown types: NORMAL, ABNORMAL, EXTERNAL, EMERGENCY
- Phases mirror startup: APP → NETWORK → LOGGING → CONFIG → FILESYSTEM → MEMORY → PLATFORM
- Idempotent and race-condition protected with atomic flags
- Register cleanup with `FUNDAMENTAL_SHUTDOWN_REGISTER(phase, cleanup_function)`

### **Networking**
- Asynchronous TCP/UDP interface with non-blocking operations
- TCP: connect, send, receive_exact, close with connection pooling
- UDP: fire-and-forget datagram send
- Address parsing and formatting for IPv4/IPv6
- All operations return `AsyncResult` for async await pattern
- Configurable buffer sizes via config module

---

## AI Agent Skills

This project includes specialized skills for AI coding agents (Opencode, Claude Code) that provide copy-paste examples for common Fundamental Library operations.

### Available Skills

| Skill | Description |
|-------|-------------|
| **[File I/O]**(.opencode/skills/fundamental-file-io.md) | Read, write, append files, stream-based I/O |
| **[Memory]**(.opencode/skills/fundamental-memory.md) | Allocate, free, copy, fill, compare memory |
| **[Console]**(.opencode/skills/fundamental-console.md) | Output text, progress bars, error messages |
| **[Directory]**(.opencode/skills/fundamental-directory.md) | Create, list, remove directories, iterate files |
| **[String]**(.opencode/skills/fundamental-string.md) | Copy, join, template, convert, compare strings |
| **[Collections]**(.opencode/skills/fundamental-collections.md) | Arrays, hashmaps, sets, red-black trees |
| **[Async]**(.opencode/skills/fundamental-async.md) | Await results, poll status, spawn processes |
| **[Config]**(.opencode/skills/fundamental-config.md) | Load configuration, cascade sources, get values |
| **[Logging]**(.opencode/skills/fundamental-logging.md) | Configure logging, log at different levels, template messages |
| **[Network]**(.opencode/skills/fundamental-network.md) | TCP connect/send/receive, UDP send |
| **[Startup/Shutdown]**(.opencode/skills/fundamental-lifecycle.md) | Register init/cleanup functions, manage lifecycle |
| **[Index]**(.opencode/skills/fundamental-skills-index.md) | Central index with cross-references |

### Using Skills

**For AI Agents:** When implementing Fundamental Library code:

1. **Identify the task**: "I need to read a file"
2. **Find the skill**: See table above or check the index
3. **Copy the pattern**: Use the example as a template
4. **Adapt to context**: Modify paths, sizes, error handling as needed

**Example Workflow:**
```
User: "Read a config file and parse it"

Agent workflow:
1. Load fundamental-file-io.md for file reading pattern
2. Load fundamental-memory.md for buffer allocation
3. Load fundamental-string.md for string parsing or templating
4. Combine patterns into working code
```

### Skill Design Principles

All skills follow these patterns:
- **Allocate → Operate → Check Error → Use → Cleanup** - Consistent flow
- **Error handling mandatory** - Every example shows error checking
- **Memory safety** - Every allocation has corresponding free
- **Cross-references** - Skills link to related skills for discovery

For more details, see the [Skills Index](.opencode/skills/fundamental-skills-index.md).

## Building

### Prerequisites

- GCC compiler (MinGW on Windows)
- Linux kernel headers (for Linux builds)
- Windows SDK (for Windows builds)

### Compilation

#### Windows

**Full Test Suite:**
```batch
run-tests-windows-amd64.bat
```

**Individual Component:**
```batch
cd tests\memory
.\build-windows-amd64.bat
.\test.exe
```

**Code Formatting:**
```batch
code-format.bat
```

#### Linux

**Individual Component:**
```bash
cd tests/memory
./build-linux-amd64.sh
./test
```

**Code Formatting:**
```bash
./code-format.sh
```

### Test Organization

Each module has its own test directory under `tests/`:
- Navigate to test directory
- Run platform-specific build script (`build-windows-amd64.bat` or `build-linux-amd64.sh`)
- Execute `test.exe` (Windows) or `./test` (Linux)
- Exit code indicates success (0) or failure (non-zero)

### Architecture-Specific Builds

Platform-specific code is isolated in `arch/<module>/<platform>/`:
- `linux-amd64/` - Linux x86_64 implementations
- `windows-amd64/` - Windows x86_64 implementations

Build scripts automatically select the appropriate architecture implementation.

## Error Handling

All functions that can fail return result types:

```
// Success case
MemoryResult result = fun_memory_allocate(size);
if (fun_error_is_ok(result.error)) {
    Memory ptr = result.value;
    // Use ptr...
}

// Error case
if (fun_error_is_error(result.error)) {
    printf("Error %d: %s\n", result.error.code, result.error.message);
}
```

## Contributing

1. **Follow naming conventions** - All functions must use `fun_` prefix with descriptive names
2. **Implement caller-allocated pattern** - Functions must not allocate memory internally
3. **Add comprehensive tests** - Each function requires corresponding test cases
4. **Update documentation** - Include usage examples for new functionality
5. **Follow Linux kernel patterns** - When in doubt, reference kernel implementations

### Code Style

- Use descriptive variable names
- Implement comprehensive error handling
- Add inline documentation for complex operations
- Follow existing code organization patterns

## License

MIT License

## Acknowledgments

- Linux kernel developers for architectural inspiration
- Platform-specific optimization techniques from various open source projects

## Roadmap

### Planned Modules

| Module | Capabilities | Status |
|--------|-------------|--------|
| **Configuration** | Cascading config from CLI, env, INI files | Complete |
| **Logging** | Centralized logging with levels, timestamps, multiple outputs | Complete |
| **Startup Framework** | Ordered initialization phases with fail-fast | Complete |
| **Shutdown Framework** | Coordinated cleanup in reverse init order | Complete |
| **Networking** | TCP/UDP async socket API | Complete (TCP/UDP basic) |
| **Time/Date** | `fun_time_now()`, `fun_time_sleep()`, timestamp formatting | Proposed |
| **Random Numbers** | PRNG with seeding, `fun_random_u32/u64()`, bounded ranges | Proposed |
| **Sorting** | Generic array sorting, binary search, comparison functions | Proposed |
| **Threading** | Thread creation, mutex/locks, atomic operations | Proposed |

All planned modules will follow the same design principles: zero stdlib dependencies, explicit error handling, caller-allocated memory, and cross-platform support.

### Infrastructure

- [ ] ARM64 architecture support (Linux and Darwin)
- [ ] Comprehensive benchmarking suite
- [ ] Integration examples and documentation
- [ ] Thread safety extensions for collections
- [ ] Advanced stream features (seeking, buffering strategies)
- [ ] Additional collection types (linked lists, queues, stacks)
- [ ] `fun_memory_compare()` - Memory comparison function to replace memcmp (needed for collections)
- [ ] HTTP client built on networking module
- [ ] TSV parsing module (in progress)

## Building Without Stdlib (Zero-Stdlib Mode)

To compile with `-nostdlib` flag for true zero-stdlib binaries:

### Windows (MinGW)
```batch
gcc -nostdlib -fno-builtin -mno-stack-arg-probe ^
    -Wl,--subsystem,console ^
    your_code.c ^
    -lkernel32 -lntdll
```

**Important flags:**
- `-mno-stack-arg-probe` - Disables ___chkstk_ms calls (stack probing)
- `-lntdll` - Provides low-level Windows runtime (NtCreateFile, etc.)
- `-lkernel32` - Windows API (CreateFile, etc.)

### Linux
```bash
gcc -nostdlib -fno-builtin -static \
    your_code.c \
    -lc
```

**Note:** All stdlib functions (memcpy, memset, etc.) are implemented in `arch/*/` using custom implementations.
