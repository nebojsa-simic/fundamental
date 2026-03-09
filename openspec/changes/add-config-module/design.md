## Context

The Fundamental Library provides building blocks for zero-stdlib CLI applications but lacks unified configuration management. Applications must manually handle command-line parsing, environment variable access, and INI file parsing across platform-specific code. The library already has:
- String module with conversion and templating
- Hashmap for key-value storage
- File module for INI file reading
- Startup module with argc/argv parsing in arch layer
- Error handling system with Result types

This change adds a config module that unifies these sources with a cascading priority model.

## Goals / Non-Goals

**Goals:**
- Provide single API for configuration from CLI args, environment variables, and INI files
- Support string, int64, and bool value types with type-safe accessors
- Implement lazy validation (errors at point-of-use, not load time)
- Use `--config:key=value` CLI prefix, `{APP}_` env prefix, flat INI format
- Platform-agnostic INI parsing using existing file module
- Ergonomic `get_or_default()` for optional values
- Follow Go-style explicit error handling patterns

**Non-Goals:**
- Schema validation at load time (deferred to point-of-use)
- Nested INI sections (flat dotted keys only)
- Array/list value types (v1 is scalar only)
- Float/double types (can add in future)
- Hot reloading or config watching
- Custom INI file paths (always `{app}.ini` in executable directory)
- Case-insensitive key matching

## Decisions

### 1. Lazy Validation Over Schema
**Decision:** Validate at point-of-use, not at load time

**Rationale:**
- Matches existing library patterns (explicit error handling everywhere)
- More flexible for conditional requirements
- Simpler API surface
- Can add schema helper later without breaking changes

**Alternatives Considered:**
- Schema validation at load: More explicit but rigid, adds complexity
- Hybrid approach: Good middle ground but adds API surface

### 2. Cascade Priority: CLI → Env → INI
**Decision:** Command-line highest, environment middle, INI lowest

**Rationale:**
- Matches industry standard (12-factor app pattern)
- Allows INI as defaults, env for deployment, CLI for overrides
- Intuitive for users

### 3. Flat INI with Dotted Keys
**Decision:** No sections, use `database.host` instead of `[database] host`

**Rationale:**
- Simpler parser implementation
- Matches env var and CLI key format
- Hierarchical enough for most use cases
- Consistent across all three sources

### 4. INI Parser in src/, Not arch/
**Decision:** Platform-agnostic INI parsing using file module

**Rationale:**
- INI format is universal, not platform-specific
- File module already handles platform differences
- Reduces code duplication
- Only env var access needs arch/ implementation

### 5. Go-Style Error Handling
**Decision:** Result types with explicit error checks

**Rationale:**
- Consistent with existing library patterns
- Clear distinction between required and optional config
- No hidden failures or exceptions

### 6. App Name for Prefix
**Decision:** Explicit app_name parameter determines env prefix

**Rationale:**
- Predictable and controllable
- Avoids executable name normalization issues
- Clear mapping: `myapp` → `MYAPP_*`

## Risks / Trade-offs

**Risk:** Users expect sections in INI files  
→ **Mitigation:** Document flat format clearly, can add sections in v2 if needed

**Risk:** Dotted keys may conflict with existing INI conventions  
→ **Mitration:** Dots are valid in INI keys, just less common than sections

**Risk:** Env var prefix collisions if app name is generic  
→ **Mitigation:** User controls app name, can use unique prefix like `mycompany-myapp`

**Risk:** No float support limits use cases  
→ **Mitigation:** Strings can represent floats, user converts; add float type in v2

**Trade-off:** Simpler API vs. less explicit contracts  
→ Lazy validation is simpler but requirements are implicit in code

**Trade-off:** Fixed INI location vs. flexibility  
→ Always `{app}.ini` in executable dir is simple but less flexible; can add custom path in v2

## Migration Plan

Not applicable - this is a new module with no existing code to migrate.

## Open Questions

None - design is fully specified and ready for implementation.
