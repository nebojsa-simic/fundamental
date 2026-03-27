## Why

The library has no built-in serialization for messages. Building distributed systems requires implementing custom serialization layers, leading to inconsistent approaches and potential bugs in every project.

## What Changes

- Add `fun_transport` library for message serialization
- Support multiple wire formats (binary, text-based)
- Provide schema definition language for message types
- Generate serialization code from schemas

## Capabilities

### New Capabilities
- `transport-serial`: Binary serialization with compact encoding
- `transport-schema`: Schema definition language for message types
- `transport-codegen`: Code generation for serialization from schemas
- `transport-streaming`: Streaming serialization for large payloads

### Modified Capabilities
- `tsv`: Existing TSV module may be integrated into transport library

## Impact

- New library: `include/fundamental/transport/`
- Requires code generation tooling
- Provides consistent serialization across all projects
- Enables efficient message queue and RPC implementations
