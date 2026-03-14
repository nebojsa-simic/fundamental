## Context

AI coding agents (Opencode, Claude Code) are used to help developers build applications with Fundamental Library. Currently, agents must infer library usage patterns from code alone, leading to:
- Inconsistent error handling patterns
- Memory management mistakes (forgetting to free)
- Incorrect module choices for tasks
- Reinventing solutions that already exist

The existing `fundamental-expert` skill provides comprehensive knowledge but is verbose for common tasks. Agents need quick-reference skills for specific operations.

Current skill structure:
- Skills located in `.opencode/skills/`
- Each skill has `SKILL.md` with frontmatter and markdown content
- Skills are loaded by agents contextually

## Goals / Non-Goals

**Goals:**
- Create focused, task-oriented skills for common operations
- Each skill covers one domain: file I/O, memory, console, etc.
- Include copy-paste examples agents can adapt
- Show error handling, memory management, cleanup in every example
- Enable agents to quickly find correct patterns
- Reduce hallucination of non-existent APIs

**Non-Goals:**
- No replacement for `fundamental-expert` skill (complementary)
- No API changes to the library itself
- No exhaustive function documentation (link to headers for that)
- No tutorial-style explanations (concise, example-driven)

## Decisions

### Decision 1: One Skill Per Domain
**Choice:** Create separate skill files for each major library domain

**Structure:**
- `.opencode/skills/fundamental-file-io.md` - File operations
- `.opencode/skills/fundamental-directory.md` - Directory operations
- `.opencode/skills/fundamental-memory.md` - Memory management
- `.opencode/skills/fundamental-console.md` - Console output
- `.opencode/skills/fundamental-string.md` - String operations
- `.opencode/skills/fundamental-collections.md` - Data structures
- `.opencode/skills/fundamental-async.md` - Async operations
- `.opencode/skills/fundamental-config.md` - Configuration

**Rationale:**
- Agents can load only relevant skills for context
- Easier to maintain and update individual skills
- Developers can reference specific skills
- Matches how developers think about tasks ("I need to read a file")

**Alternatives considered:**
- Single monolithic skill: Too large, hard to navigate
- One skill per function: Too granular, loses context
- Organize by module directory: Doesn't match user mental model

### Decision 2: Example-First Format
**Choice:** Lead with working code examples, minimize prose

**Format:**
```markdown
## Task: Read a File

```c
#include "file/file.h"
#include "memory/memory.h"

// Allocate buffer
MemoryResult mem_result = fun_memory_allocate(4096);
if (fun_error_is_error(mem_result.error)) {
    return 1;
}

// Read file
AsyncResult read_result = fun_read_file_in_memory((Read){
    .file_path = "path/to/file.txt",
    .output = mem_result.value,
    .bytes_to_read = 4096
});
fun_async_await(&read_result);

// Use file contents...

// Cleanup
voidResult free_result = fun_memory_free(&mem_result.value);
```
```

**Rationale:**
- Agents copy patterns directly into generated code
- Visual learners grasp patterns faster from examples
- Reduces ambiguity in implementation
- Shows complete flow including error handling and cleanup

**Alternatives considered:**
- API reference style: Too dry, requires interpretation
- Tutorial style: Too verbose for quick reference
- Diagrams/flowcharts: Not executable, agents can't copy

### Decision 3: Consistent Example Structure
**Choice:** Every example follows: Allocate → Operation → Error Check → Use → Cleanup

**Rationale:**
- Reinforces library patterns through repetition
- Ensures examples are production-ready
- Teaches proper resource management by example
- Prevents memory leaks in agent-generated code

**Alternatives considered:**
- Minimal examples (just the call): Missing critical error handling
- Full application context: Too verbose, distracts from pattern

### Decision 4: Cross-References Between Skills
**Choice:** Skills reference each other for related operations

**Example:**
- File I/O skill links to Memory skill for allocation patterns
- Console skill links to String skill for formatting
- Collections skill links to Memory skill for cleanup

**Rationale:**
- Skills remain focused but connected
- Agents discover related patterns naturally
- Reduces duplication across skills

## Risks / Trade-offs

**[Skills become outdated]** → Library API changes but skills don't
- **Mitigation:** Skills are markdown files - easy to update; add version metadata; include "last updated" date

**[Too many skills to manage]** → Proliferation of skill files
- **Mitigation:** Limit to major domains; combine related operations; index skills in README

**[Agents ignore skills]** → Agents fall back to general knowledge
- **Mitigation:** Reference skills in system prompts; name skills clearly; make examples highly visible

**[Examples become copy-paste cargo cult]** → Developers use patterns without understanding
- **Mitigation:** Include brief explanations of why pattern matters; link to deeper documentation

**[Skill overlap causes confusion]** → Multiple skills cover same operation
- **Mitigation:** Clear skill boundaries; cross-references instead of duplication; single source of truth per operation

## Migration Plan

**Phase 1: Create Core Skills**
1. Create skill files for 8 major domains (file, directory, memory, console, string, collections, async, config)
2. Populate each with 3-5 common task examples
3. Include error handling and cleanup in all examples

**Phase 2: Agent Integration**
1. Update Opencode/Claude Code system prompts to reference skills
2. Test skill discovery with common queries
3. Refine skill naming based on agent behavior

**Phase 3: Documentation**
1. Add skills index to project README
2. Link skills from module headers
3. Create "Skills" section in CONTRIBUTING.md

No migration needed for existing code - skills are documentation layer only.

## Open Questions

1. **Skill format standardization**: Should we follow a specific skill format (like the existing SKILL.md frontmatter)? Decision: Yes, use YAML frontmatter with name, description, compatibility.

2. **Skill location**: Keep in `.opencode/skills/` or move to `docs/skills/` for broader access? Decision: Keep in `.opencode/skills/` for agent visibility, symlink or copy to docs if needed.

3. **Examples with Path type**: Path type refactoring is pending. Should skills use current String API or future Path API? Decision: Use current String API, add migration note when Path type lands.
