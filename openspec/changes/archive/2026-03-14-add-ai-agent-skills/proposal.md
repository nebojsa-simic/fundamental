## Why

AI coding agents (Opencode, Claude Code) need reusable skills that demonstrate proper Fundamental Library usage patterns. Currently, agents must figure out library APIs from scratch for common tasks like file I/O, memory management, and directory iteration. Well-documented skills reduce errors, ensure best practices, and accelerate agent-assisted development.

## What Changes

- Create Opencode/Claude Code skills for common library operations
- Add skills for file operations: read file, write file, check file exists, iterate directory
- Add skills for memory management: allocate, free, memory operations
- Add skills for console output: progress bars, formatted output
- Add skills for string operations: conversion, templating, manipulation
- Add skills for data structures: collections, hashmaps, sets, red-black trees
- Add skills for async operations: await results, handle async status
- No breaking changes - purely additive documentation

## Capabilities

### New Capabilities
- `agent-skill-file-io`: File reading, writing, appending, existence checks with proper error handling
- `agent-skill-directory`: Directory listing, iteration, creating, removing with recursive operations
- `agent-skill-memory`: Memory allocation, deallocation, copy, fill, compare operations
- `agent-skill-console`: Progress bar rendering, console output, formatted text display
- `agent-skill-string`: String conversion, templating, joining, trimming operations
- `agent-skill-collections`: Dynamic array usage, iteration, insertion, removal patterns
- `agent-skill-hashmap`: Key-value storage, lookup, iteration, deletion patterns
- `agent-skill-async`: Async result handling, awaiting completion, status checking

### Modified Capabilities

## Impact

- **New skill files**: `.opencode/skills/fundamental-file-io.md`, `.opencode/skills/fundamental-memory.md`, etc.
- **AI agent workflow**: Agents can reference skills when implementing library operations
- **Code quality**: Consistent usage patterns across agent-generated code
- **Learning curve**: New users can study skills as usage examples
- **No API changes**: Library remains unchanged - skills are documentation layer only
- **Agent configuration**: May update agent system prompts to reference available skills
