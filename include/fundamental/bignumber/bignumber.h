#ifndef LIBRARY_BIGNUMBER_H
#define LIBRARY_BIGNUMBER_H

#include <stddef.h>
#include <stdint.h>

/*
 * bignumber — variable-width unsigned big-number arithmetic.
 *
 * Represented as an array of 32-bit limbs, little-endian (d[0] = LSB).
 * Maximum width: 32 limbs (1024 bits). The `limbs` field tracks the
 * active width.
 *
 * All functions are infallible (no memory allocation, no errors).
 * Operate on stack-allocated BigNumber values by value.
 *
 * Typical widths:
 *   128-bit  → 4 limbs
 *   256-bit  → 8 limbs   (secp256r1 / P-256)
 *   512-bit  → 16 limbs  (intermediate products)
 *
 * Modular arithmetic works for any prime modulus: a fast NIST P-256
 * reduction is used when the modulus equals the secp256r1 prime, and a
 * generic bit-by-bit long-division reduction is used otherwise.
 */

#define BIGNUMBER_MAX_LIMBS 32

typedef struct {
	uint32_t d[BIGNUMBER_MAX_LIMBS];
	uint32_t limbs;
} BigNumber;

/* ------------------------------------------------------------------
 * Construction
 *
 * `limbs` must be 1..BIGNUMBER_MAX_LIMBS; it is clamped to that range.
 * ------------------------------------------------------------------ */

BigNumber fun_bignumber_zero(uint32_t limbs);
BigNumber fun_bignumber_from_u32(uint32_t limbs, uint32_t v);
BigNumber fun_bignumber_from_bytes(uint32_t limbs, const uint8_t *bytes);
void fun_bignumber_to_bytes(const BigNumber *a, uint8_t *bytes);

/* ------------------------------------------------------------------
 * Queries
 * ------------------------------------------------------------------ */

int fun_bignumber_is_zero(const BigNumber *a);
int fun_bignumber_is_one(const BigNumber *a);
int fun_bignumber_is_odd(const BigNumber *a);
int fun_bignumber_compare(const BigNumber *a, const BigNumber *b);

/* ------------------------------------------------------------------
 * Basic arithmetic (modulo 2^W)
 * ------------------------------------------------------------------ */

BigNumber fun_bignumber_add(const BigNumber *a, const BigNumber *b);
BigNumber fun_bignumber_subtract(const BigNumber *a, const BigNumber *b);
BigNumber fun_bignumber_shift_right_one(const BigNumber *a);

/* ------------------------------------------------------------------
 * Modular arithmetic
 *
 * All results satisfy 0 <= result < modulus.
 * Caller guarantees a, b < modulus and all values share the same
 * limb count.
 *
 * fun_bignumber_modular_inverse requires a != 0 and a prime modulus
 * (it uses Fermat's little theorem, a^(modulus-2) mod modulus). For
 * a == 0 it returns 0, which is not a valid inverse; callers must
 * ensure a != 0.
 * ------------------------------------------------------------------ */

BigNumber fun_bignumber_modular_add(const BigNumber *a, const BigNumber *b,
									const BigNumber *modulus);
BigNumber fun_bignumber_modular_subtract(const BigNumber *a, const BigNumber *b,
										 const BigNumber *modulus);
BigNumber fun_bignumber_modular_multiply(const BigNumber *a, const BigNumber *b,
										 const BigNumber *modulus);
BigNumber fun_bignumber_modular_inverse(const BigNumber *a,
										const BigNumber *modulus);

/* ------------------------------------------------------------------
 * Wide multiplication
 *
 * Full double-width product: out = a * b.
 * `out` must point to at least (a->limbs + b->limbs) uint64_t values.
 * Each entry holds a single 32-bit limb of the product (only lower 32
 * bits used).
 * ------------------------------------------------------------------ */

void fun_bignumber_multiply_wide(uint64_t *out, const BigNumber *a,
								 const BigNumber *b);

#endif /* LIBRARY_BIGNUMBER_H */