## Why

The Stream API requires seven required struct fields just to open a stream. Line-by-line reading, delimited protocols, and stream composition require custom implementations in every project.

## What Changes

- Simplify stream creation with builder pattern
- Add line and delimited reading utilities
- Support stream composition and transformation
- Add better error reporting for stream operations

## Capabilities

### New Capabilities
- `stream-builder`: Fluent API for stream configuration
- `stream-lines`: Line-by-line reading with automatic buffering
- `stream-delimited`: Custom delimiter-based reading
- `stream-compose`: Stream chaining and transformation utilities

### Modified Capabilities
- `stream`: Existing stream functions will gain simpler variants

## Impact

- New convenience functions alongside existing low-level API
- Existing code remains compatible
- Reduces boilerplate for common stream operations
- Enables protocol implementations (HTTP, custom delimited protocols)
