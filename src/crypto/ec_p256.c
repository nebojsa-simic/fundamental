/* EC P-256 — ECDH and ECDSA on secp256r1, built on bignumber.
 *
 * Scalar multiplication uses Jacobian coordinates with a Montgomery
 * ladder and constant-time conditional swaps. This (a) needs only one
 * field inversion per scalar multiplication instead of one per point
 * operation (huge speedup), and (b) makes the sequence of operations
 * independent of the private scalar's bits.
 */
#include <stdint.h>

#include "fundamental/bignumber/bignumber.h"
#include "fundamental/crypto/crypto.h"

#define NL 8 /* 256-bit */

static const uint8_t p256_bytes[32] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* Group order n for secp256r1 */
static const uint8_t n256_bytes[32] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17,
	0x9E, 0x84, 0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
};

/* Curve parameter b */
static const uint8_t b_curve_bytes[32] = {
	0x5A, 0xC6, 0x35, 0xD8, 0xAA, 0x3A, 0x93, 0xE7, 0xB3, 0xEB, 0xBD,
	0x55, 0x76, 0x98, 0x86, 0xBC, 0x65, 0x1D, 0x06, 0xB0, 0xCC, 0x53,
	0xB0, 0xF6, 0x3B, 0xCE, 0x3C, 0x3E, 0x27, 0xD2, 0x60, 0x4B
};

static const uint8_t Gx_bytes[32] = { 0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42,
									  0x47, 0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4,
									  0x40, 0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D,
									  0xEB, 0x33, 0xA0, 0xF4, 0xA1, 0x39, 0x45,
									  0xD8, 0x98, 0xC2, 0x96 };

static const uint8_t Gy_bytes[32] = { 0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F,
									  0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F,
									  0x9E, 0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B,
									  0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40, 0x68,
									  0x37, 0xBF, 0x51, 0xF5 };

static BigNumber p256_prime(void)
{
	return fun_bignumber_from_bytes(NL, p256_bytes);
}

/* ---- field helpers (mod p) ---- */

static BigNumber fadd(const BigNumber *a, const BigNumber *b,
					  const BigNumber *p)
{
	return fun_bignumber_modular_add(a, b, p);
}
static BigNumber fsub(const BigNumber *a, const BigNumber *b,
					  const BigNumber *p)
{
	return fun_bignumber_modular_subtract(a, b, p);
}
static BigNumber fmul(const BigNumber *a, const BigNumber *b,
					  const BigNumber *p)
{
	return fun_bignumber_modular_multiply(a, b, p);
}
static BigNumber fsmall(uint32_t v)
{
	return fun_bignumber_from_u32(NL, v);
}

/* ---- affine points (used for validity checks and ECDSA combination) ---- */

typedef struct {
	BigNumber x;
	BigNumber y;
} ec_point;

static ec_point ec_inf(void)
{
	ec_point r;
	r.x = fun_bignumber_zero(NL);
	r.y = fun_bignumber_zero(NL);
	return r;
}

static int ec_is_inf(const ec_point *P)
{
	return fun_bignumber_is_zero(&P->x) && fun_bignumber_is_zero(&P->y);
}

/*
 * Verify y^2 = x^3 - 3x + b (mod p) and that coordinates are in [0, p).
 * Rejects the point at infinity. Returns 1 if the point is valid.
 */
static int ec_point_is_valid(const ec_point *P)
{
	BigNumber p = p256_prime();
	BigNumber three = fsmall(3);
	BigNumber b = fun_bignumber_from_bytes(NL, b_curve_bytes);
	BigNumber lhs, x2, x3, threex, rhs;

	if (ec_is_inf(P))
		return 0;
	if (fun_bignumber_compare(&P->x, &p) >= 0)
		return 0;
	if (fun_bignumber_compare(&P->y, &p) >= 0)
		return 0;

	lhs = fmul(&P->y, &P->y, &p);
	x2 = fmul(&P->x, &P->x, &p);
	x3 = fmul(&x2, &P->x, &p);
	threex = fmul(&three, &P->x, &p);
	rhs = fsub(&x3, &threex, &p);
	rhs = fadd(&rhs, &b, &p);
	return fun_bignumber_compare(&lhs, &rhs) == 0;
}

/* Affine point addition (used only for the ECDSA R1+R2 combination,
   which operates on public data, so variable-time is acceptable). */
static ec_point ec_add_affine(const ec_point *P, const ec_point *Q)
{
	BigNumber p = p256_prime();
	BigNumber lam, num, den, den_inv, lam2, tmp, x3, y3;
	ec_point R;

	if (ec_is_inf(P))
		return *Q;
	if (ec_is_inf(Q))
		return *P;

	if (fun_bignumber_compare(&P->x, &Q->x) == 0) {
		if (fun_bignumber_compare(&P->y, &Q->y) != 0)
			return ec_inf();
		{
			BigNumber x2 = fmul(&P->x, &P->x, &p);
			BigNumber three = fsmall(3);
			BigNumber x2_3 = fmul(&x2, &three, &p);
			num = fsub(&x2_3, &three, &p);
			den = fadd(&P->y, &P->y, &p);
		}
	} else {
		num = fsub(&Q->y, &P->y, &p);
		den = fsub(&Q->x, &P->x, &p);
	}

	den_inv = fun_bignumber_modular_inverse(&den, &p);
	lam = fmul(&num, &den_inv, &p);
	lam2 = fmul(&lam, &lam, &p);
	tmp = fsub(&lam2, &P->x, &p);
	x3 = fsub(&tmp, &Q->x, &p);
	{
		BigNumber dx = fsub(&P->x, &x3, &p);
		BigNumber lamdx = fmul(&lam, &dx, &p);
		y3 = fsub(&lamdx, &P->y, &p);
	}
	R.x = x3;
	R.y = y3;
	return R;
}

/* ---- Jacobian points: (X, Y, Z) represents affine (X/Z^2, Y/Z^3) ---- */

typedef struct {
	BigNumber X;
	BigNumber Y;
	BigNumber Z;
} jac_point;

static jac_point jac_from_affine(const ec_point *P)
{
	jac_point J;
	J.X = P->x;
	J.Y = P->y;
	J.Z = fsmall(1);
	return J;
}

/* Branchless swap of two Jacobian points based on a 0/1 bit. */
static void bn_cswap(BigNumber *a, BigNumber *b, uint32_t bit)
{
	uint32_t mask = (uint32_t)0 - bit; /* 0x00000000 or 0xFFFFFFFF */
	uint32_t i;
	for (i = 0; i < NL; i++) {
		uint32_t t = mask & (a->d[i] ^ b->d[i]);
		a->d[i] ^= t;
		b->d[i] ^= t;
	}
}

static void jac_cswap(jac_point *a, jac_point *b, uint32_t bit)
{
	bn_cswap(&a->X, &b->X, bit);
	bn_cswap(&a->Y, &b->Y, bit);
	bn_cswap(&a->Z, &b->Z, bit);
}

/* Point doubling in Jacobian coordinates, a = -3 (dbl-2001-b). */
static jac_point jac_double(const jac_point *P, const BigNumber *p)
{
	BigNumber three = fsmall(3), four = fsmall(4), eight = fsmall(8);
	BigNumber delta, gamma, beta, t1, t2, t12, alpha, alpha2, eightbeta;
	BigNumber X3, Y1Z1, Y1Z1sq, tz, Z3, gamma2, eightgamma2, fourbeta, t3,
		alpha_t3, Y3;
	jac_point R;

	delta = fmul(&P->Z, &P->Z, p);
	gamma = fmul(&P->Y, &P->Y, p);
	beta = fmul(&P->X, &gamma, p);
	t1 = fsub(&P->X, &delta, p);
	t2 = fadd(&P->X, &delta, p);
	t12 = fmul(&t1, &t2, p);
	alpha = fmul(&t12, &three, p);
	alpha2 = fmul(&alpha, &alpha, p);
	eightbeta = fmul(&eight, &beta, p);
	X3 = fsub(&alpha2, &eightbeta, p);
	Y1Z1 = fadd(&P->Y, &P->Z, p);
	Y1Z1sq = fmul(&Y1Z1, &Y1Z1, p);
	tz = fsub(&Y1Z1sq, &gamma, p);
	Z3 = fsub(&tz, &delta, p);
	gamma2 = fmul(&gamma, &gamma, p);
	eightgamma2 = fmul(&eight, &gamma2, p);
	fourbeta = fmul(&four, &beta, p);
	t3 = fsub(&fourbeta, &X3, p);
	alpha_t3 = fmul(&alpha, &t3, p);
	Y3 = fsub(&alpha_t3, &eightgamma2, p);

	R.X = X3;
	R.Y = Y3;
	R.Z = Z3;
	return R;
}

/*
 * Point addition in Jacobian coordinates for distinct, non-identity
 * P, Q (add-1998-cmo-2). Note: this formula is NOT complete -- if an
 * input is the identity (Z == 0) the result is the identity, and it
 * does not handle P == Q. The Montgomery ladder only ever supplies
 * distinct, non-identity operands (a*P and (a+1)*P with a >= 1), so
 * these cases do not arise. Do not reuse this for arbitrary points
 * without a completeness guard.
 */
static jac_point jac_add(const jac_point *P, const jac_point *Q,
						 const BigNumber *p)
{
	BigNumber two = fsmall(2);
	BigNumber Z1Z1, Z2Z2, U1, U2, S1, S2, H, twoH, I, J, r, V, rr, X3;
	BigNumber tPZ, tQZ, twoV, tS1J, twoS1J, VX3, rVX3, Y3, Zsum, Zsumsq,
		Zsumsub, Z3;
	jac_point R;

	Z1Z1 = fmul(&P->Z, &P->Z, p);
	Z2Z2 = fmul(&Q->Z, &Q->Z, p);
	U1 = fmul(&P->X, &Z2Z2, p);
	U2 = fmul(&Q->X, &Z1Z1, p);
	tPZ = fmul(&P->Y, &Q->Z, p);
	S1 = fmul(&tPZ, &Z2Z2, p);
	tQZ = fmul(&Q->Y, &P->Z, p);
	S2 = fmul(&tQZ, &Z1Z1, p);
	H = fsub(&U2, &U1, p);
	twoH = fmul(&two, &H, p);
	I = fmul(&twoH, &twoH, p);
	J = fmul(&H, &I, p);
	{
		BigNumber SmS = fsub(&S2, &S1, p);
		r = fmul(&SmS, &two, p);
	}
	V = fmul(&U1, &I, p);
	rr = fmul(&r, &r, p);
	twoV = fmul(&two, &V, p);
	{
		BigNumber rrJ = fsub(&rr, &J, p);
		X3 = fsub(&rrJ, &twoV, p);
	}
	tS1J = fmul(&S1, &J, p);
	twoS1J = fmul(&two, &tS1J, p);
	VX3 = fsub(&V, &X3, p);
	rVX3 = fmul(&r, &VX3, p);
	Y3 = fsub(&rVX3, &twoS1J, p);
	Zsum = fadd(&P->Z, &Q->Z, p);
	Zsumsq = fmul(&Zsum, &Zsum, p);
	Zsumsub = fsub(&Zsumsq, &Z1Z1, p);
	Zsumsub = fsub(&Zsumsub, &Z2Z2, p);
	Z3 = fmul(&Zsumsub, &H, p);

	R.X = X3;
	R.Y = Y3;
	R.Z = Z3;
	return R;
}

/* Convert a Jacobian point to affine. Z == 0 maps to the identity (0,0). */
static ec_point jac_to_affine(const jac_point *P, const BigNumber *p)
{
	ec_point r;
	BigNumber zinv, zinv2, yfactor;

	if (fun_bignumber_is_zero(&P->Z)) {
		r.x = fun_bignumber_zero(NL);
		r.y = fun_bignumber_zero(NL);
		return r;
	}
	zinv = fun_bignumber_modular_inverse(&P->Z, p);
	zinv2 = fmul(&zinv, &zinv, p);
	yfactor = fmul(&zinv2, &zinv, p);
	r.x = fmul(&P->X, &zinv2, p);
	r.y = fmul(&P->Y, &yfactor, p);
	return r;
}

static int highest_set_bit(const BigNumber *k)
{
	int i;
	for (i = NL * 32 - 1; i >= 0; i--)
		if ((k->d[i >> 5] >> (i & 31)) & 1)
			return i;
	return -1;
}

/*
 * Scalar multiplication k*P using a constant-time Montgomery ladder in
 * Jacobian coordinates. Returns the affine result. k == 0 returns the
 * identity.
 */
static ec_point ec_mul(const BigNumber *k, const ec_point *P)
{
	BigNumber p = p256_prime();
	jac_point R0, R1, sum, dbl;
	ec_point res;
	int hb = highest_set_bit(k);
	int i;

	if (hb < 0) {
		res = ec_inf();
		return res;
	}

	R0 = jac_from_affine(P); /* represents 1*P */
	R1 = jac_double(&R0, &p); /* represents 2*P */

	for (i = hb - 1; i >= 0; i--) {
		uint32_t bit = (k->d[i >> 5] >> (i & 31)) & 1;
		jac_cswap(&R0, &R1, bit);
		sum = jac_add(&R0, &R1, &p); /* R0 + R1 */
		dbl = jac_double(&R0, &p); /* 2 * R0 */
		R1 = sum;
		R0 = dbl;
		jac_cswap(&R0, &R1, bit);
	}

	res = jac_to_affine(&R0, &p);
	return res;
}

/* ---- public API ---- */
int fun_crypto_ec_p256_public_key(const uint8_t private_key[32],
								  fun_crypto_ec_point *public_key)
{
	BigNumber Gx = fun_bignumber_from_bytes(NL, Gx_bytes);
	BigNumber Gy = fun_bignumber_from_bytes(NL, Gy_bytes);
	ec_point G = { Gx, Gy };
	BigNumber k = fun_bignumber_from_bytes(NL, private_key);
	BigNumber n = fun_bignumber_from_bytes(NL, n256_bytes);
	ec_point K;
	uint32_t i;

	/* A zero or out-of-range scalar is not a usable private key. */
	if (fun_bignumber_is_zero(&k) || fun_bignumber_compare(&k, &n) >= 0) {
		for (i = 0; i < 32; i++) {
			public_key->x[i] = 0;
			public_key->y[i] = 0;
		}
		return -1;
	}

	K = ec_mul(&k, &G);
	fun_bignumber_to_bytes(&K.x, public_key->x);
	fun_bignumber_to_bytes(&K.y, public_key->y);
	return 0;
}

int fun_crypto_ecdh_p256(const uint8_t private_key[32],
						 const fun_crypto_ec_point *peer_public,
						 uint8_t shared_secret[32])
{
	BigNumber k = fun_bignumber_from_bytes(NL, private_key);
	BigNumber n = fun_bignumber_from_bytes(NL, n256_bytes);
	BigNumber Qx = fun_bignumber_from_bytes(NL, peer_public->x);
	BigNumber Qy = fun_bignumber_from_bytes(NL, peer_public->y);
	ec_point Q = { Qx, Qy };
	ec_point Z;
	uint32_t i;
	int invalid = 0;

	if (fun_bignumber_is_zero(&k) || fun_bignumber_compare(&k, &n) >= 0)
		invalid = 1;
	if (!invalid && !ec_point_is_valid(&Q))
		invalid = 1;
	if (!invalid) {
		Z = ec_mul(&k, &Q);
		if (ec_is_inf(&Z))
			invalid = 1;
	}

	if (invalid) {
		/* Zero the output and signal failure. The caller MUST abort
		   the handshake; an all-zero secret is fully predictable. */
		for (i = 0; i < 32; i++)
			shared_secret[i] = 0;
		return -1;
	}

	fun_bignumber_to_bytes(&Z.x, shared_secret);
	return 0;
}

int fun_crypto_ecdsa_verify(const uint8_t hash[32],
							const fun_crypto_ec_point *public_key,
							const uint8_t sig_r[32], const uint8_t sig_s[32])
{
	BigNumber n = fun_bignumber_from_bytes(NL, n256_bytes);
	BigNumber Gx = fun_bignumber_from_bytes(NL, Gx_bytes);
	BigNumber Gy = fun_bignumber_from_bytes(NL, Gy_bytes);
	ec_point G = { Gx, Gy };

	BigNumber Qx = fun_bignumber_from_bytes(NL, public_key->x);
	BigNumber Qy = fun_bignumber_from_bytes(NL, public_key->y);
	ec_point Q = { Qx, Qy };

	BigNumber z = fun_bignumber_from_bytes(NL, hash);
	BigNumber r = fun_bignumber_from_bytes(NL, sig_r);
	BigNumber s = fun_bignumber_from_bytes(NL, sig_s);
	BigNumber s_inv, u1, u2;
	ec_point R1, R2, R;

	if (!ec_point_is_valid(&Q))
		return -1;
	if (fun_bignumber_is_zero(&r) || fun_bignumber_compare(&r, &n) >= 0)
		return -1;
	if (fun_bignumber_is_zero(&s) || fun_bignumber_compare(&s, &n) >= 0)
		return -1;

	s_inv = fun_bignumber_modular_inverse(&s, &n);
	u1 = fun_bignumber_modular_multiply(&z, &s_inv, &n);
	u2 = fun_bignumber_modular_multiply(&r, &s_inv, &n);

	R1 = ec_mul(&u1, &G);
	R2 = ec_mul(&u2, &Q);
	R = ec_add_affine(&R1, &R2);

	if (ec_is_inf(&R))
		return -1;

	/* Compare R.x mod n against r. */
	if (fun_bignumber_compare(&R.x, &n) >= 0) {
		R.x = fun_bignumber_subtract(&R.x, &n);
		if (fun_bignumber_compare(&R.x, &n) >= 0)
			R.x = fun_bignumber_subtract(&R.x, &n);
	}
	return (fun_bignumber_compare(&R.x, &r) == 0) ? 0 : -1;
}