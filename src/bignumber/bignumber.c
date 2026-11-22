/* bignumber — variable-width unsigned big-number arithmetic.  No stdlib. */
#include "fundamental/bignumber/bignumber.h"

/* ------------------------------------------------------------------
 * Internal constants and helpers
 * ------------------------------------------------------------------ */

/* secp256r1 prime p (big-endian bytes) */
static const uint8_t p256_prime_bytes[32] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* delta = 2^256 mod p = 2^224 - 2^192 - 2^96 + 1 (big-endian bytes) */
static const uint8_t p256_delta_bytes[32] = {
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

static uint32_t clamp_limbs(uint32_t limbs)
{
	if (limbs == 0)
		return 1;
	if (limbs > BIGNUMBER_MAX_LIMBS)
		return BIGNUMBER_MAX_LIMBS;
	return limbs;
}

/* ------------------------------------------------------------------
 * Construction
 * ------------------------------------------------------------------ */

BigNumber fun_bignumber_zero(uint32_t limbs)
{
	BigNumber r;
	uint32_t i;
	r.limbs = clamp_limbs(limbs);
	for (i = 0; i < r.limbs; i++)
		r.d[i] = 0;
	for (i = r.limbs; i < BIGNUMBER_MAX_LIMBS; i++)
		r.d[i] = 0;
	return r;
}

BigNumber fun_bignumber_from_u32(uint32_t limbs, uint32_t v)
{
	BigNumber r = fun_bignumber_zero(limbs);
	r.d[0] = v;
	return r;
}

BigNumber fun_bignumber_from_bytes(uint32_t limbs, const uint8_t *bytes)
{
	BigNumber r = fun_bignumber_zero(limbs);
	uint32_t i;
	for (i = 0; i < r.limbs; i++) {
		uint32_t idx = (r.limbs - 1 - i) * 4;
		r.d[i] = ((uint32_t)bytes[idx] << 24) |
				 ((uint32_t)bytes[idx + 1] << 16) |
				 ((uint32_t)bytes[idx + 2] << 8) | (uint32_t)bytes[idx + 3];
	}
	return r;
}

void fun_bignumber_to_bytes(const BigNumber *a, uint8_t *bytes)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t i;
	for (i = 0; i < n; i++) {
		uint32_t w = a->d[n - 1 - i];
		bytes[i * 4] = (uint8_t)(w >> 24);
		bytes[i * 4 + 1] = (uint8_t)(w >> 16);
		bytes[i * 4 + 2] = (uint8_t)(w >> 8);
		bytes[i * 4 + 3] = (uint8_t)w;
	}
}

/* ------------------------------------------------------------------
 * Queries
 * ------------------------------------------------------------------ */

int fun_bignumber_is_zero(const BigNumber *a)
{
	uint32_t i, n = clamp_limbs(a->limbs);
	for (i = 0; i < n; i++)
		if (a->d[i])
			return 0;
	return 1;
}

int fun_bignumber_is_one(const BigNumber *a)
{
	uint32_t i, n = clamp_limbs(a->limbs);
	if (a->d[0] != 1)
		return 0;
	for (i = 1; i < n; i++)
		if (a->d[i])
			return 0;
	return 1;
}

int fun_bignumber_is_odd(const BigNumber *a)
{
	return a->d[0] & 1;
}

int fun_bignumber_compare(const BigNumber *a, const BigNumber *b)
{
	uint32_t al = clamp_limbs(a->limbs);
	uint32_t bl = clamp_limbs(b->limbs);
	uint32_t max = al > bl ? al : bl;
	uint32_t i;
	/* Compare limbs from high to low, treating missing limbs as zero */
	for (i = max; i-- > 0;) {
		uint32_t av = (i < al) ? a->d[i] : 0;
		uint32_t bv = (i < bl) ? b->d[i] : 0;
		if (av > bv)
			return 1;
		if (av < bv)
			return -1;
	}
	return 0;
}

/* ------------------------------------------------------------------
 * Basic arithmetic
 * ------------------------------------------------------------------ */

BigNumber fun_bignumber_add(const BigNumber *a, const BigNumber *b)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t bl = clamp_limbs(b->limbs);
	BigNumber r = fun_bignumber_zero(n);
	uint64_t carry = 0;
	uint32_t i;
	for (i = 0; i < n; i++) {
		uint64_t bi = (i < bl) ? b->d[i] : 0;
		uint64_t s = (uint64_t)a->d[i] + bi + carry;
		r.d[i] = (uint32_t)s;
		carry = s >> 32;
	}
	return r;
}

BigNumber fun_bignumber_subtract(const BigNumber *a, const BigNumber *b)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t bl = clamp_limbs(b->limbs);
	BigNumber r = fun_bignumber_zero(n);
	uint32_t borrow = 0;
	uint32_t i;
	for (i = 0; i < n; i++) {
		uint64_t ai = a->d[i];
		uint64_t bi = (i < bl) ? b->d[i] : 0;
		if (borrow) {
			if (ai == 0)
				ai = 0xFFFFFFFF;
			else {
				ai--;
				borrow = 0;
			}
		}
		if (ai < bi) {
			borrow = 1;
			r.d[i] = (uint32_t)(ai + ((uint64_t)1 << 32) - bi);
		} else {
			r.d[i] = (uint32_t)(ai - bi);
		}
	}
	return r;
}

BigNumber fun_bignumber_shift_right_one(const BigNumber *a)
{
	uint32_t n = clamp_limbs(a->limbs);
	BigNumber r = fun_bignumber_zero(n);
	uint32_t carry = 0;
	uint32_t i;
	for (i = n; i-- > 0;) {
		uint32_t nc = a->d[i] & 1;
		r.d[i] = (a->d[i] >> 1) | (carry << 31);
		carry = nc;
	}
	return r;
}

/* ------------------------------------------------------------------
 * Modular arithmetic
 *
 * The add/subtract operations are branchless: they compute both the
 * "keep" and "reduced" candidates and select with a limb mask derived
 * from the carry/borrow flags, so the control flow does not depend on
 * secret operand values.
 * ------------------------------------------------------------------ */

/* r = a + b (mod 2^256); *carry_out = carry out of the top limb. */
static BigNumber bn_add_carry(const BigNumber *a, const BigNumber *b,
							  uint32_t *carry_out)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t bl = clamp_limbs(b->limbs);
	BigNumber r = fun_bignumber_zero(n);
	uint64_t carry = 0;
	uint32_t i;
	for (i = 0; i < n; i++) {
		uint64_t bi = (i < bl) ? b->d[i] : 0;
		uint64_t s = (uint64_t)a->d[i] + bi + carry;
		r.d[i] = (uint32_t)s;
		carry = s >> 32;
	}
	*carry_out = (uint32_t)carry;
	return r;
}

/* r = a - b (mod 2^256); *borrow_out = 1 if a < b. */
static BigNumber bn_sub_borrow(const BigNumber *a, const BigNumber *b,
							   uint32_t *borrow_out)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t bl = clamp_limbs(b->limbs);
	BigNumber r = fun_bignumber_zero(n);
	uint32_t borrow = 0;
	uint32_t i;
	for (i = 0; i < n; i++) {
		uint64_t ai = a->d[i];
		uint64_t bi = (i < bl) ? b->d[i] : 0;
		if (borrow) {
			if (ai == 0)
				ai = 0xFFFFFFFF;
			else {
				ai--;
				borrow = 0;
			}
		}
		if (ai < bi) {
			borrow = 1;
			r.d[i] = (uint32_t)(ai + ((uint64_t)1 << 32) - bi);
		} else {
			r.d[i] = (uint32_t)(ai - bi);
		}
	}
	*borrow_out = borrow;
	return r;
}

BigNumber fun_bignumber_modular_add(const BigNumber *a, const BigNumber *b,
									const BigNumber *modulus)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t carry, borrow, sel, mask, i;
	BigNumber sum, diff, r;

	sum = bn_add_carry(a, b, &carry);
	diff = bn_sub_borrow(&sum, modulus, &borrow);
	/* Subtract when the sum overflowed 2^256 (carry) or is >= modulus
	   (i.e. no borrow from sum - modulus). */
	sel = carry | (borrow ^ 1u);
	mask = (uint32_t)0 - sel;
	r = fun_bignumber_zero(n);
	for (i = 0; i < n; i++)
		r.d[i] = sum.d[i] ^ (mask & (sum.d[i] ^ diff.d[i]));
	return r;
}

BigNumber fun_bignumber_modular_subtract(const BigNumber *a, const BigNumber *b,
										 const BigNumber *modulus)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t borrow, carry, mask, i;
	BigNumber diff, cand, r;

	diff = bn_sub_borrow(a, b, &borrow);
	cand = bn_add_carry(&diff, modulus, &carry);
	/* If a < b (borrow), add the modulus back. */
	mask = (uint32_t)0 - borrow;
	(void)carry;
	r = fun_bignumber_zero(n);
	for (i = 0; i < n; i++)
		r.d[i] = diff.d[i] ^ (mask & (diff.d[i] ^ cand.d[i]));
	return r;
}

/* ------------------------------------------------------------------
 * Wide multiplication
 * ------------------------------------------------------------------ */

void fun_bignumber_multiply_wide(uint64_t *out, const BigNumber *a,
								 const BigNumber *b)
{
	uint32_t n = clamp_limbs(a->limbs);
	uint32_t nb = clamp_limbs(b->limbs);
	uint32_t n2 = n + nb;
	uint32_t i;
	for (i = 0; i < n2; i++)
		out[i] = 0;
	for (i = 0; i < n; i++) {
		uint64_t carry = 0;
		uint32_t j;
		for (j = 0; j < nb; j++) {
			uint64_t t =
				(uint64_t)a->d[i] * (uint64_t)b->d[j] + out[i + j] + carry;
			out[i + j] = t & 0xFFFFFFFFULL;
			carry = t >> 32;
		}
		out[i + nb] += carry;
	}
}

/* ------------------------------------------------------------------
 * Reduction — generic (any modulus) and fast P-256 path
 * ------------------------------------------------------------------ */

static int is_p256_prime(const BigNumber *m)
{
	uint32_t i;
	if (m->limbs != 8)
		return 0;
	for (i = 0; i < 8; i++) {
		uint32_t idx = (7 - i) * 4;
		uint32_t w = ((uint32_t)p256_prime_bytes[idx] << 24) |
					 ((uint32_t)p256_prime_bytes[idx + 1] << 16) |
					 ((uint32_t)p256_prime_bytes[idx + 2] << 8) |
					 (uint32_t)p256_prime_bytes[idx + 3];
		if (m->d[i] != w)
			return 0;
	}
	return 1;
}

/*
 * Generic reduction of a 2n-limb product modulo an n-limb modulus.
 * Uses bit-by-bit long division: process product bits from MSB to LSB,
 * maintaining a running remainder r < modulus.  Each step shifts r left,
 * brings in one product bit, then conditionally subtracts the modulus.
 * Works for arbitrary moduli (not just P-256).
 */
static BigNumber reduce_generic(const uint64_t *prod, const BigNumber *modulus)
{
	uint32_t n = clamp_limbs(modulus->limbs);
	BigNumber r = fun_bignumber_zero(n);
	int total_bits = (int)(n * 2 * 32);
	int b;

	for (b = total_bits - 1; b >= 0; b--) {
		uint32_t in_bit = (uint32_t)((prod[b / 32] >> (b % 32)) & 1);
		uint32_t carry = in_bit;
		uint32_t borrow, sel, mask, i;
		BigNumber diff;

		for (i = 0; i < n; i++) {
			uint32_t nc = r.d[i] >> 31;
			r.d[i] = (r.d[i] << 1) | carry;
			carry = nc;
		}
		/* Branchless conditional subtract: subtract when the shifted
		   remainder overflowed the top bit (carry) or is >= modulus
		   (no borrow from r - modulus). */
		diff = bn_sub_borrow(&r, modulus, &borrow);
		sel = carry | (borrow ^ 1u);
		mask = (uint32_t)0 - sel;
		for (i = 0; i < n; i++)
			r.d[i] = r.d[i] ^ (mask & (r.d[i] ^ diff.d[i]));
	}
	return r;
}

/*
 * Fast NIST P-256 reduction.
 * p = 2^256 - delta, delta = 2^256 - p.
 * x = hi*2^256 + lo; x mod p = hi*delta + lo (applied iteratively).
 */
static BigNumber reduce_p256(const uint64_t *prod, const BigNumber *modulus)
{
	uint32_t n = clamp_limbs(modulus->limbs);
	BigNumber delta = fun_bignumber_from_bytes(8, p256_delta_bytes);
	BigNumber lo = fun_bignumber_zero(n);
	BigNumber hi = fun_bignumber_zero(n);
	uint32_t i;
	uint32_t iter;

	/* Caller guarantees is_p256_prime(modulus), so n == 8. */
	for (i = 0; i < n; i++) {
		lo.d[i] = (uint32_t)prod[i];
		hi.d[i] = (uint32_t)prod[i + n];
	}

	/* Fixed iteration count (no secret-dependent early exit). Once hi
	   reaches zero the extra rounds are no-ops (0*delta + lo == lo). */
	for (iter = 0; iter < 32; iter++) {
		uint64_t acc[2 * BIGNUMBER_MAX_LIMBS];
		uint32_t n2 = n * 2;
		uint64_t carry = 0;

		fun_bignumber_multiply_wide(acc, &hi, &delta);
		for (i = n2; i < 2 * BIGNUMBER_MAX_LIMBS; i++)
			acc[i] = 0;

		for (i = 0; i < n; i++) {
			uint64_t s = acc[i] + (uint64_t)lo.d[i] + carry;
			acc[i] = s & 0xFFFFFFFFULL;
			carry = s >> 32;
		}
		for (i = n; i < n2 && carry; i++) {
			uint64_t s = acc[i] + carry;
			acc[i] = s & 0xFFFFFFFFULL;
			carry = s >> 32;
		}

		for (i = 0; i < n; i++) {
			lo.d[i] = (uint32_t)acc[i];
			hi.d[i] = (uint32_t)acc[i + n];
		}
	}

	/* Final branchless conditional subtract (lo < 2*modulus). */
	{
		uint32_t borrow, mask, i;
		BigNumber diff = bn_sub_borrow(&lo, modulus, &borrow);
		mask = (uint32_t)0 - (borrow ^ 1u);
		for (i = 0; i < n; i++)
			lo.d[i] = lo.d[i] ^ (mask & (lo.d[i] ^ diff.d[i]));
	}
	return lo;
}

BigNumber fun_bignumber_modular_multiply(const BigNumber *a, const BigNumber *b,
										 const BigNumber *modulus)
{
	uint64_t prod[2 * BIGNUMBER_MAX_LIMBS];
	uint32_t i;

	for (i = 0; i < 2 * BIGNUMBER_MAX_LIMBS; i++)
		prod[i] = 0;

	fun_bignumber_multiply_wide(prod, a, b);

	if (is_p256_prime(modulus))
		return reduce_p256(prod, modulus);
	return reduce_generic(prod, modulus);
}

/* ------------------------------------------------------------------
 * Modular inverse — Fermat's Little Theorem: a^(p-2) mod p
 * (valid for prime moduli; P-256 prime and group order are both prime)
 * ------------------------------------------------------------------ */

BigNumber fun_bignumber_modular_inverse(const BigNumber *a,
										const BigNumber *modulus)
{
	BigNumber two = fun_bignumber_from_u32(modulus->limbs, 2);
	BigNumber exp = fun_bignumber_subtract(modulus, &two);
	BigNumber result = fun_bignumber_from_u32(modulus->limbs, 1);
	BigNumber base = *a;

	while (!fun_bignumber_is_zero(&exp)) {
		if (fun_bignumber_is_odd(&exp))
			result = fun_bignumber_modular_multiply(&result, &base, modulus);
		base = fun_bignumber_modular_multiply(&base, &base, modulus);
		exp = fun_bignumber_shift_right_one(&exp);
	}
	return result;
}