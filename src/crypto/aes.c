/* AES-128 block cipher per FIPS-197 */
#include <stdint.h>
#include "fundamental/crypto/crypto.h"

/* Volatile wipe that the optimizer cannot elide. */
static void aes_secure_zero(volatile void *p, size_t n)
{
	volatile uint8_t *b = (volatile uint8_t *)p;
	size_t i;
	for (i = 0; i < n; i++)
		b[i] = 0;
}

/*
 * Constant-time S-box.  Computed algebraically (GF(2^8) inverse via
 * x^254 followed by the affine transform) so that no secret-dependent
 * memory indexing (cache-timing) occurs.  Slower than a table but
 * side-channel safe.
 */
static uint8_t ct_gmul(uint8_t a, uint8_t b)
{
	uint8_t prod = 0;
	int i;
	for (i = 0; i < 8; i++) {
		uint8_t bmask = (uint8_t)(0 - (b & 1));
		uint8_t hi = (uint8_t)(0 - (a >> 7));
		prod ^= (uint8_t)(a & bmask);
		a = (uint8_t)((a << 1) ^ (0x1b & hi));
		b >>= 1;
	}
	return prod;
}

static uint8_t ct_rotl8(uint8_t x, int n)
{
	return (uint8_t)((x << n) | (x >> (8 - n)));
}

static uint8_t aes_sbox_ct(uint8_t x)
{
	uint8_t x2 = ct_gmul(x, x);
	uint8_t x4 = ct_gmul(x2, x2);
	uint8_t x8 = ct_gmul(x4, x4);
	uint8_t x16 = ct_gmul(x8, x8);
	uint8_t x32 = ct_gmul(x16, x16);
	uint8_t x64 = ct_gmul(x32, x32);
	uint8_t x128 = ct_gmul(x64, x64);
	uint8_t p = ct_gmul(x128, x64);
	uint8_t inv;
	p = ct_gmul(p, x32);
	p = ct_gmul(p, x16);
	p = ct_gmul(p, x8);
	p = ct_gmul(p, x4);
	p = ct_gmul(p, x2);
	inv = p;
	return (uint8_t)(inv ^ ct_rotl8(inv, 1) ^ ct_rotl8(inv, 2) ^
					 ct_rotl8(inv, 3) ^ ct_rotl8(inv, 4) ^ 0x63);
}

static const uint32_t RCON[10] = { 0x01, 0x02, 0x04, 0x08, 0x10,
								   0x20, 0x40, 0x80, 0x1b, 0x36 };

static uint8_t xtime(uint8_t b)
{
	return (uint8_t)((b << 1) ^ ((b & 0x80) ? 0x1b : 0));
}

void fun_crypto_aes128_encrypt(const uint8_t key[16], const uint8_t in[16],
							   uint8_t out[16])
{
	int Nk = 4, Nr = 10, Nb = 4;
	uint32_t w[44];
	uint8_t s[4][4];
	int i, r, c;

	/* Key expansion */
	for (i = 0; i < Nk; i++) {
		w[i] = ((uint32_t)key[4 * i] << 24) | ((uint32_t)key[4 * i + 1] << 16) |
			   ((uint32_t)key[4 * i + 2] << 8) | (uint32_t)key[4 * i + 3];
	}
	for (i = Nk; i < Nb * (Nr + 1); i++) {
		uint32_t t = w[i - 1];
		if (i % Nk == 0) {
			t = ((uint32_t)aes_sbox_ct((t >> 16) & 0xFF) << 24) |
				((uint32_t)aes_sbox_ct((t >> 8) & 0xFF) << 16) |
				((uint32_t)aes_sbox_ct(t & 0xFF) << 8) |
				(uint32_t)aes_sbox_ct(t >> 24);
			t ^= (uint32_t)RCON[i / Nk - 1] << 24;
		}
		w[i] = w[i - Nk] ^ t;
	}

	/* State init */
	for (r = 0; r < 4; r++)
		for (c = 0; c < 4; c++)
			s[r][c] = in[4 * c + r];

	/* AddRoundKey */
	for (c = 0; c < 4; c++) {
		uint32_t wk = w[c];
		s[0][c] ^= (uint8_t)(wk >> 24);
		s[1][c] ^= (uint8_t)(wk >> 16);
		s[2][c] ^= (uint8_t)(wk >> 8);
		s[3][c] ^= (uint8_t)wk;
	}

	/* Rounds 1..Nr-1 */
	for (r = 1; r < Nr; r++) {
		/* SubBytes */
		for (i = 0; i < 4; i++)
			for (c = 0; c < 4; c++)
				s[i][c] = aes_sbox_ct(s[i][c]);

		/* ShiftRows */
		uint8_t t = s[1][0];
		s[1][0] = s[1][1];
		s[1][1] = s[1][2];
		s[1][2] = s[1][3];
		s[1][3] = t;
		t = s[2][0];
		s[2][0] = s[2][2];
		s[2][2] = t;
		t = s[2][1];
		s[2][1] = s[2][3];
		s[2][3] = t;
		t = s[3][3];
		s[3][3] = s[3][2];
		s[3][2] = s[3][1];
		s[3][1] = s[3][0];
		s[3][0] = t;

		/* MixColumns */
		for (c = 0; c < 4; c++) {
			uint8_t a0 = s[0][c], a1 = s[1][c], a2 = s[2][c], a3 = s[3][c];
			s[0][c] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
			s[1][c] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
			s[2][c] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
			s[3][c] = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
		}

		/* AddRoundKey */
		for (c = 0; c < 4; c++) {
			uint32_t wk = w[4 * r + c];
			s[0][c] ^= (uint8_t)(wk >> 24);
			s[1][c] ^= (uint8_t)(wk >> 16);
			s[2][c] ^= (uint8_t)(wk >> 8);
			s[3][c] ^= (uint8_t)wk;
		}
	}

	/* Final round: SubBytes + ShiftRows + AddRoundKey */
	for (i = 0; i < 4; i++)
		for (c = 0; c < 4; c++)
			s[i][c] = aes_sbox_ct(s[i][c]);

	uint8_t t = s[1][0];
	s[1][0] = s[1][1];
	s[1][1] = s[1][2];
	s[1][2] = s[1][3];
	s[1][3] = t;
	t = s[2][0];
	s[2][0] = s[2][2];
	s[2][2] = t;
	t = s[2][1];
	s[2][1] = s[2][3];
	s[2][3] = t;
	t = s[3][3];
	s[3][3] = s[3][2];
	s[3][2] = s[3][1];
	s[3][1] = s[3][0];
	s[3][0] = t;

	for (c = 0; c < 4; c++) {
		uint32_t wk = w[4 * Nr + c];
		s[0][c] ^= (uint8_t)(wk >> 24);
		s[1][c] ^= (uint8_t)(wk >> 16);
		s[2][c] ^= (uint8_t)(wk >> 8);
		s[3][c] ^= (uint8_t)wk;
	}

	/* Output */
	for (r = 0; r < 4; r++)
		for (c = 0; c < 4; c++)
			out[4 * c + r] = s[r][c];

	/* Scrub key schedule and state from the stack. */
	aes_secure_zero(w, sizeof(w));
	aes_secure_zero(s, sizeof(s));
}