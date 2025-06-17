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
├── arch/               # Platform-specific implementations
│   ├── file/          # File I/O implementations per platform
│   └── memory/        # Memory management per architecture
├── include/           # Public API headers
│   ├── async/         # Async operation primitives
│   ├── error/         # Error handling system
│   ├── file/          # File I/O interface
│   ├── memory/        # Memory management interface
│   └── string/        # String operations interface
├── src/               # Core implementations
│   ├── async/         # Async scheduler and utilities
│   └── string/        # String operations (conversion, templating, validation)
└── tests/             # Unit tests for all modules
```

## Features

### **Error Handling System**
- Comprehensive result types for all operations
- Explicit error propagation without exceptions
- Standardized error codes and messages

### **Memory Management**
- Direct syscall-based allocation (Linux)
- Platform-specific optimized implementations
- Caller-controlled allocation patterns

### **String Operations**
- Complete string manipulation suite
- Template system with type-safe parameter substitution
- Conversion utilities (int, double, pointer to string)
- In-place and out-of-place operations

### **Async File I/O**
- Multiple I/O strategies: standard, memory-mapped, ring-based
- Platform-optimized implementations
- Non-blocking operation support

### **Architecture Support**
- Linux AMD64 (primary)
- Windows AMD64 (in development)
- Extensible architecture system

## Quick Start

### Basic String Operations

```
#include "string/string.h"
#include "memory/memory.h"

// Caller allocates output buffer
char output;
String source = "Hello, World!";

// Copy string
fun_string_copy(source, output);

// Get length
StringLength len = fun_string_length(source);

// Convert integer to string
fun_string_from_int(42, 10, output);
```

### Memory Management

```
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

```
#include "string/string.h"

char output;
String template = "Hello #{name}, you have #{age} messages";

StringTemplateParam params[] = {
    { "name", { .stringValue = "Alice" } },
    { "age", { .intValue = 42 } }
};

fun_string_template(template, params, 2, output);
// Result: "Hello Alice, you have 42 messages"
```

### Async File Operations

```
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

## Building

### Prerequisites

- GCC or Clang compiler
- Linux kernel headers (for Linux builds)
- Windows SDK (for Windows builds)

### Compilation

TODO

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

- [ ] Complete Windows memory implementation
- [ ] Add more architecture support (ARM64)
- [ ] Implement network I/O module
- [ ] Add comprehensive benchmarking suite
- [ ] Create integration examples
