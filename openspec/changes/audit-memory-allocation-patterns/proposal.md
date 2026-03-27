## Why

The documentation states "caller-allocated memory pattern" but several functions allocate internally. This inconsistency leads to memory leaks and confusion about ownership responsibilities.

## What Changes

- Audit all library functions for allocation behavior
- Document allocation ownership in function signatures
- Fix functions that leak internal allocations
- Add consistent allocation/deallocation patterns

## Capabilities

### New Capabilities
- `memory-audit`: Complete audit of all allocation patterns
- `memory-docs`: Clear ownership documentation in headers
- `memory-consistency`: Consistent allocation patterns across modules

### Modified Capabilities
- `memory`: Memory module documentation and patterns will be clarified
- `stream`: FileStream internal state management will be fixed
- `network`: Connection pool allocations will be documented

## Impact

- No API breaking changes expected
- Documentation improvements in all headers
- Bug fixes for memory leaks in internal allocations
- Clearer ownership model for library users
