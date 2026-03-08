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
- File locking for exclusive access
- File change notifications (platform-specific)

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

- [ ] ARM64 architecture support (Linux and Darwin)
- [ ] Network I/O module with async sockets
- [ ] Comprehensive benchmarking suite
- [ ] Integration examples and documentation
- [ ] Thread safety extensions for collections
- [ ] Advanced stream features (seeking, buffering strategies)
- [ ] Additional collection types (linked lists, queues, stacks)
