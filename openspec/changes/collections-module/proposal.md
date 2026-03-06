# Collections Module Change Proposal

## Problem Statement

The fundamental library currently lacks modern collection data structures. Developers using this library must implement lists, maps, trees, and sets manually using basic memory operations, which leads to:

- Repetitive implementation of basic data structures
- Higher potential for memory safety issues
- Violation of DRY principles 
- Inconsistent API patterns across different use cases

## Proposed Change

Add a "Collections" module that mirrors the architecture and patterns of existing modules but provides modern data structure implementations:

- Dynamic arrays (vectors) for growable sequences
- Hash maps for associative key-value storage
- Red-Black trees for ordered mappings
- Sets for unique element storage

This change maintains all existing library principles:
- Explicit error handling via ErrorResult system
- Caller-allocated memory model
- Cross-platform compatibility
- Descriptive naming conventions
- Type safety through macro-based generics

## Solution Overview

### Dynamic Arrays (Vectors)
- Homogeneous, growable array with automatic capacity management
- Push/get/set operations with bounds checking
- Typesafe through macro generation (`DEFINE_ARRAY_TYPE(int)` produces `fun_array_int_*` functions)

### Hash Maps
- Key-value associative storage with O(1) average lookup performance  
- Collision handling with linked-list chaining or specialized structures
- Support for common types (strings as keys, primitive/pointer values)

### Red-Black Trees
- Self-balancing binary search tree providing O(log n) operations
- Ordered iteration capabilities
- Suitable for ordered indexing or priority operations

### Sets
- Store unique elements with O(1) average membership check
- Built on hash map foundations to ensure uniqueness guarantee

## Scope

- [ ] Implement basic Collection interfaces
- [ ] Create platform-specific implementations for both Linux and Windows  
- [ ] Develop comprehensive test suites mapping to OpenSpec scenarios
- [ ] Update documentation with usage examples
- [ ] Implement memory-efficient storage patterns

## Success Criteria

- Collections match performance characteristics of equivalent C++/Java/JavaScript data structures
- Consistent API design with existing library patterns
- Robust memory safety matching the library's core safety promises
- Platform compatibility matching existing modules
- Comprehensive test coverage for edge cases and error conditions

## Timeline

Estimated 3-4 weeks for complete implementation, testing, and documentation.