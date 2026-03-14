## ADDED Requirements

### Requirement: Memory Management Skill Structure
The AI agent skill for memory management SHALL provide copy-paste examples for memory operations.

#### Scenario: Skill file location
- **WHEN** an AI agent needs to perform memory operations
- **THEN** the agent can find the skill at `.opencode/skills/fundamental-memory.md`

#### Scenario: Skill frontmatter
- **WHEN** the skill file is parsed
- **THEN** it contains YAML frontmatter with name, description, license, and metadata fields

### Requirement: Memory Allocation Example
The memory skill SHALL demonstrate allocating memory with proper error handling.

#### Scenario: Allocation includes error check
- **WHEN** the skill shows memory allocation
- **THEN** it checks `fun_error_is_error(mem_result.error)` immediately
- **AND** it handles allocation failure before using the memory

#### Scenario: Allocation shows size calculation
- **WHEN** the skill demonstrates buffer sizing
- **THEN** it explains how to calculate appropriate buffer sizes
- **AND** it shows common size patterns (256 for strings, 4096 for file buffers)

### Requirement: Memory Free Example
The memory skill SHALL demonstrate freeing allocated memory.

#### Scenario: Free example shows pointer handling
- **WHEN** the skill shows memory deallocation
- **THEN** it passes address of Memory variable to `fun_memory_free()`
- **AND** it explains that Memory is set to NULL after free

#### Scenario: Free includes error check
- **WHEN** the skill demonstrates freeing memory
- **THEN** it shows checking free result for errors
- **AND** it explains when free can fail (rare, but possible)

### Requirement: Memory Operations Examples
The memory skill SHALL demonstrate memory copy, fill, and compare operations.

#### Scenario: Memory copy example
- **WHEN** the skill shows memory copying
- **THEN** it uses `fun_memory_copy(destination, source, size)`
- **AND** it explains destination must be pre-allocated

#### Scenario: Memory fill example
- **WHEN** the skill shows memory initialization
- **THEN** it uses `fun_memory_fill(memory, value, size)`
- **AND** it shows common patterns (zeroing with 0, memset patterns)

#### Scenario: Memory compare example
- **WHEN** the skill shows memory comparison
- **THEN** it uses `fun_memory_compare(a, b, size)`
- **AND** it explains return value interpretation

### Requirement: Memory Safety Patterns
The memory skill SHALL demonstrate memory safety best practices.

#### Scenario: No use-after-free pattern
- **WHEN** the skill shows memory lifecycle
- **THEN** it demonstrates not using memory after free
- **AND** it explains Memory becomes NULL after successful free

#### Scenario: No double-free pattern
- **WHEN** the skill shows cleanup code
- **THEN** it demonstrates freeing each allocation exactly once
- **AND** it explains double-free consequences

#### Scenario: Buffer overflow prevention
- **WHEN** the skill shows buffer operations
- **THEN** it tracks buffer sizes explicitly
- **AND** it validates sizes before operations
