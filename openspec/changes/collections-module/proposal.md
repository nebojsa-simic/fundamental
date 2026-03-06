# Collections Module Change Proposal

## Problem Statement

The fundamental library currently lacks modern collection data structures. Developers using this library must implement lists, maps, trees, and sets manually, which leads to:

- Repetitive implementation of basic data structures
- Higher potential for memory safety issues
- Violation of DRY principles 
- Inconsistent API patterns across different use cases

The existing library provides excellent cross-platform memory management but lacks rich container data structures built on top of this foundation.

## Proposed Change

Add a "Collections" module that leverages the existing cross-platform memory management system to provide OS-agnostic data structures implemented in pure C with no OS-specific code:

- Dynamic arrays (vectors) for growable sequences
- Hash maps for associative key-value storage  
- Red-Black trees for ordered mappings
- Sets for unique element storage

**Critical Design Principle**: Collections implementations will be completely platform-agnostic (no code in arch/) and rely entirely on the library's existing `fun_memory_*` functions for all allocation needs.

This change maintains all existing library principles:
- Explicit error handling via ErrorResult system
- Cross-platform compatibility through OS-agnostic code
- Descriptive naming conventions
- Type safety through macro-based generics

## Solution Overview

### Dynamic Arrays (OS-Agnostic)
- Homogeneous, growable array with automatic capacity management
- Implemented as pure data structure code in `src/array/` 
- Push/get/set operations with bounds checking
- Typesafe through macro generation (`DEFINE_ARRAY_TYPE(int)` produces `fun_array_int_*` functions)
- All memory ops via `fun_memory_reallocate`, `fun_memory_free`, etc.

### Hash Maps (OS-Agnostic)
- OS-independent key-value associative storage using the library memory layer
- O(1) average lookup performance built on memory-management abstraction
- Collision handling through chaining with performance fallbacks
- Implemented in `src/map/` with no OS-specific implementations

### Red-Black Trees (OS-Agnostic) 
- Pure self-balancing binary search tree with O(log n) operations
- Located in `src/tree/` with no architecture-specific code
- Suitable for ordered indexing or priority operations

### Sets (OS-Agnostic)
- Unique-element storage with O(1) average membership check
- Built on hash map foundations using library memory management for correctness
- No platform-specific implementations in `arch/`

## Scope

- [ ] Implement basic Collection interfaces as pure C code with no OS dependencies
- [ ] Place all implementations in `src/collections/` (not `arch/collection/`)  
- [ ] Develop comprehensive test suites mapping to OpenSpec scenarios
- [ ] Update documentation with usage examples
- [ ] Implement memory-efficient storage patterns using existing memory module

## Success Criteria

- Collections match performance characteristics of equivalent C++/Java/JavaScript data structures while using only `fun_memory_*` operations
- Completely platform-agnostic code with identical behavior on Windows, Linux, etc.
- Consistent API design with existing library patterns
- Robust memory safety achieved via existing library error and memory systems
- Comprehensive test coverage for edge cases and error conditions across all platforms

## Timeline

Estimated 3-4 weeks for complete implementation, testing, and documentation of platform-agnostic collections.