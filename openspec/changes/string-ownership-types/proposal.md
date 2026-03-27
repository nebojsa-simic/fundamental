## Why

String handling requires manual buffer allocation and tracking ownership. Developers must pre-allocate buffers and manually track which code owns which string, leading to memory leaks and use-after-free bugs.

## What Changes

- Introduce owned `String` type with automatic lifetime management
- Add borrowed `&str` type for non-owning string references
- **BREAKING**: Raw `const char *` usage will be discouraged in favor of new types
- Compiler enforces ownership rules at compile time

## Capabilities

### New Capabilities
- `owned-string`: Heap-allocated string type with automatic cleanup
- `string-slice`: Borrowed string reference type without ownership
- `string-lifetime`: Compiler enforcement of string lifetimes and borrowing rules

### Modified Capabilities
- `string`: Existing string functions will be updated to use new ownership types

## Impact

- Requires compiler support for ownership tracking
- Existing `typedef const char *String` will be replaced
- Eliminates manual buffer size tracking for most use cases
- Breaking change for existing code using raw string pointers
