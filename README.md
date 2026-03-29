# Fundamental Library

A simple replacement for the C standard library.

## Overview

The Fundamental Library is a complete reimplementation of standard C library functionality without dependencies on the standard C library. It follows a kernel-style architecture with platform-specific optimizations and explicit error handling throughout.

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
│   ├── memory/                # Memory management per architecture
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
│   ├── memory/                # Memory management interface
│   ├── rbtree/                # Red-black tree interface
│   ├── set/                   # Set data structure interface
│   ├── stream/                # Stream I/O interface
│   └── string/                # String operations interface
├── src/                       # Core implementations
│   ├── array/                 # Dynamic array implementation
│   ├── async/                 # Async scheduler and process spawn
│   ├── config/                # Config core, INI parser, CLI parser
│   ├── console/               # Console output implementation
│   ├── filesystem/            # Path and directory operations
│   ├── hashmap/               # Hash map implementation
│   ├── rbtree/                # Red-black tree implementation
│   ├── set/                   # Set implementation
│   ├── startup/               # Cross-platform entry point
│   ├── stream/                # Stream lifecycle and file operations
│   └── string/                # String operations (conversion, templating, validation)
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

## Quick Start

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

AsyncResult result = fun_read_file_in_memory(read_params);
fun_async_await(&result);

if (result.status == ASYNC_COMPLETED) {
    // File read successfully
}
```

### File Write with Durability Guarantee

```c
#include "file/file.h"
#include "memory/memory.h"

const char *data = "critical record";
size_t len = 15;

MemoryResult buf = fun_memory_allocate(len);
memcpy(buf.value, data, len);

/* FILE_DURABILITY_FULL: fsync() ensures data and metadata reach disk */
Write params = {
    .file_path       = "/data/journal.log",
    .input           = buf.value,
    .bytes_to_write  = len,
    .offset          = 0,
    .mode            = FILE_MODE_MMAP,
    .durability_mode = FILE_DURABILITY_FULL,
};

AsyncResult result = fun_write_memory_to_file(params);
fun_async_await(&result, -1);
fun_memory_free(&buf.value);

if (result.status == ASYNC_COMPLETED) {
    /* guaranteed on disk */
}
```

| Mode | Guarantee | Use when |
|------|-----------|----------|
| `FILE_DURABILITY_ASYNC` | Page cache only | Performance-critical, loss acceptable |
| `FILE_DURABILITY_SYNC` | `msync`/`fsync` after write | Data must survive process crash |
| `FILE_DURABILITY_FULL` | `fsync` data + metadata | Data must survive power loss |

### Stream I/O

```c
#include "stream/stream.h"

// Allocate buffer for stream
MemoryResult mem_result = fun_memory_allocate(4096);
if (fun_error_is_ok(mem_result.error)) {
    // Open file for reading
    AsyncResult open_result = fun_stream_create_file_read(
        "/path/to/file.txt",
        mem_result.value,
        4096,
        FILE_MODE_STANDARD
    );
    fun_async_await(&open_result);
    
    FileStream *stream = (FileStream *)open_result.state;
    
    // Read from stream
    uint64_t bytes_read;
    AsyncResult read_result = fun_stream_read(stream, &bytes_read);
    fun_async_await(&read_result);
    
    // Check if more data available
    while (fun_stream_can_read(stream)) {
        read_result = fun_stream_read(stream, &bytes_read);
        fun_async_await(&read_result);
        // Process buffer contents...
    }
    
    // Close and destroy
    fun_stream_destroy(stream);
    fun_memory_free(&mem_result.value);
}
```

### Console Output

```c
#include "console/console.h"

// Line-buffered output (auto-flush on newline)
fun_console_write_line("Hello, World!");

// Buffered output without newline
fun_console_write("Processing: ");
fun_console_write("50%");
fun_console_flush();  // Force output

// Error output (unbuffered, immediate)
fun_console_error_line("An error occurred!");
```

### Filesystem Operations

```c
#include "filesystem/filesystem.h"

// Create directory with parents
ErrorResult mkdir_result = fun_filesystem_create_directory("/tmp/test/nested");

// List directory contents
char output[4096];
ErrorResult list_result = fun_filesystem_list_directory("/tmp", output);

// Path utilities
char path[512];
fun_path_join("/home/user", "documents", path);
fun_path_normalize("/home/user/../user/./docs", path);
fun_path_get_filename("/home/user/file.txt", path);
```

### Collections - Dynamic Arrays

```c
#include "array/array.h"

// Define type-safe array operations for int
DEFINE_ARRAY_TYPE(int)

// Create array with initial capacity
intArrayResult create_result = fun_array_int_create(16);
if (fun_error_is_ok(create_result.error)) {
    intArray array = create_result.value;
    
    // Push elements
    fun_array_int_push(&array, 42);
    fun_array_int_push(&array, 100);
    
    // Get element
    int value = fun_array_int_get(&array, 0);
    
    // Get size
    size_t count = fun_array_int_size(&array);
    
    // Destroy when done
    fun_array_int_destroy(&array);
}
```

### Collections - Hash Maps

```c
#include "hashmap/hashmap.h"

// Define type-safe hashmap for int keys and int values
// For other types, use DEFINE_HASHMAP_TYPE(KeyType, ValueType)
DEFINE_HASHMAP_TYPE(int, int)

// Create hashmap
intintHashMapResult create_result = fun_hashmap_int_int_create(16);
if (fun_error_is_ok(create_result.error)) {
    intintHashMap map = create_result.value;
    
    // Insert key-value pairs
    fun_hashmap_int_int_put(&map, 1, 42);
    fun_hashmap_int_int_put(&map, 2, 100);
    
    // Retrieve value
    int value = fun_hashmap_int_int_get(&map, 1);  // value = 42
    
    // Check if key exists
    bool contains;
    fun_hashmap_int_int_contains(&map, 1, &contains);
    
    // Destroy when done
    fun_hashmap_int_int_destroy(&map);
}
```

### Async Process Spawn

```c
#include "async/async.h"

// Spawn process
const char *args[] = { "echo", "Hello from subprocess", NULL };
AsyncResult result = fun_async_process_spawn("echo", args, NULL);

// Wait for completion
fun_async_await(&result);

// Get output
size_t out_len;
const char *stdout_data = fun_process_get_stdout(&result, &out_len);

// Get exit code
int exit_code = fun_process_get_exit_code(&result);

// Clean up
fun_process_free(&result);
```

### Configuration Management

```c
#include "config/config.h"

// Load config (cascades: CLI args → env vars → myapp.ini)
ConfigResult cfg_result = fun_config_load("myapp", argc, argv);
if (fun_error_is_error(cfg_result.error)) {
    fun_console_error_line("Failed to load config");
    return 1;
}
Config config = cfg_result.value;

// Get a required string value
StringResult host_result = fun_config_get_string(&config, "database.host");
if (fun_error_is_error(host_result.error)) {
    fun_console_error_line("database.host is required");
    fun_config_destroy(&config);
    return 1;
}
// host_result.value is a char* pointing into config's internal buffers

// Get an optional integer with a default
int64_tResult port_result = fun_config_get_int_or_default(&config, "database.port", 5432);
int64_t port = port_result.value;

// Get an optional boolean with a default
boolResult debug_result = fun_config_get_bool_or_default(&config, "debug", false);
bool debug = debug_result.value;

// Check existence without type conversion
boolResult has_result = fun_config_has(&config, "some.key");
if (has_result.value) {
    // key exists in any source
}

// Cleanup (frees all internal memory)
fun_config_destroy(&config);
```

**INI file format** (`myapp.ini` in same directory as executable):
```ini
; comment lines start with ; or #
database.host = localhost
database.port = 5432
debug = false
app.name = "My Application"
```

**Environment variable** `MYAPP_DATABASE_HOST` overrides INI `database.host`.
**CLI argument** `--config:database.host=prod.db.example.com` overrides all.

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
| **Time/Date** | `fun_time_now()`, `fun_time_sleep()`, timestamp formatting | Proposed |
| **Random Numbers** | PRNG with seeding, `fun_random_u32/u64()`, bounded ranges | Proposed |
| **Sorting** | Generic array sorting, binary search, comparison functions | Proposed |
| **Networking** | Socket API, TCP/UDP, HTTP client | Proposed |
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
