## Why

HashMap type generation requires verbose macro invocations that generate hundreds of lines of code. Debugging macro expansion errors is painful, and the current approach doesn't scale for complex key/value types.

## What Changes

- Simplify macro interface with better type inference
- Reduce generated code footprint
- Improve compiler error messages for HashMap operations
- Add iterator support for key-value traversal

## Capabilities

### New Capabilities
- `hashmap-iterators`: Forward and backward iteration over HashMap entries
- `hashmap-shorthand`: Simplified macro syntax with type inference
- `hashmap-debug`: Better error messages and debug output for HashMap operations

### Modified Capabilities
- `hashmap`: Core HashMap macros will be simplified

## Impact

- Existing `DEFINE_HASHMAP_TYPE()` macros remain compatible
- New shorthand syntax optional for new code
- Requires compiler improvements for better type inference
- Reduces compilation time by generating less boilerplate
