// Fundamental Library - Architecture Memory Utilities
// Low-level memory operations for arch code (no stdlib dependencies)
// These are intentionally simple - no error handling, no Memory wrappers

#ifndef ARCH_MEMORY_UTILS_H
#define ARCH_MEMORY_UTILS_H

#include <stddef.h>
#include <stdint.h>

// ============================================================================
// Inline Memory Operations (for use in arch code only)
// ============================================================================

// Simple byte copy - replaces memcpy()
static inline void arch_memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *d = (uint8_t *)dest;
	const uint8_t *s = (const uint8_t *)src;
	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	}
}

// Simple byte set - replaces memset()
static inline void arch_memset(void *s, int c, size_t n)
{
	uint8_t *p = (uint8_t *)s;
	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t)c;
	}
}

// Simple byte compare - replaces memcmp()
static inline int arch_memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t *p1 = (const uint8_t *)s1;
	const uint8_t *p2 = (const uint8_t *)s2;
	for (size_t i = 0; i < n; i++) {
		if (p1[i] != p2[i]) {
			return p1[i] - p2[i];
		}
	}
	return 0;
}

#endif // ARCH_MEMORY_UTILS_H
