/* AES-128-GCM per NIST SP 800-38D */
#include <stdint.h>
#include "fundamental/crypto/crypto.h"

/* Volatile wipe that the optimizer cannot elide. */
static void gcm_secure_zero(volatile void *p, size_t n)
{
	volatile uint8_t *b = (volatile uint8_t *)p;
	size_t i;
	for (i = 0; i < n; i++)
		b[i] = 0;
}

static void gf128_mul(const uint8_t x[16], const uint8_t y[16], uint8_t z[16])
{
	uint8_t v[16], r[16];
	uint32_t i, j;

	for (i = 0; i < 16; i++) {
		v[i] = y[i];
		r[i] = 0;
	}
	/* Branchless: both the conditional XOR and the reduction are
	   driven by masks so no data-dependent branching occurs. */
	for (i = 0; i < 128; i++) {
		uint8_t bit = (uint8_t)((x[i / 8] >> (7 - (i % 8))) & 1);
		uint8_t bmask = (uint8_t)(0 - bit);
		uint8_t carry;
		uint8_t cmask;
		for (j = 0; j < 16; j++)
			r[j] ^= (uint8_t)(v[j] & bmask);
		carry = v[15] & 1;
		cmask = (uint8_t)(0 - carry);
		for (j = 15; j > 0; j--)
			v[j] = (uint8_t)((v[j] >> 1) | (v[j - 1] << 7));
		v[0] >>= 1;
		v[0] ^= (uint8_t)(0xE1 & cmask);
	}
	for (i = 0; i < 16; i++)
		z[i] = r[i];
}

static void xor16(uint8_t *d, const uint8_t *a, const uint8_t *b)
{
	uint32_t i;
	for (i = 0; i < 16; i++)
		d[i] = (uint8_t)(a[i] ^ b[i]);
}

static void inc32(uint8_t blk[16])
{
	int i;
	for (i = 15; i >= 12; i--) {
		blk[i]++;
		if (blk[i] != 0)
			break;
	}
}

void fun_crypto_aes128_gcm_encrypt(const uint8_t key[16], const uint8_t *iv,
								   size_t iv_len, const uint8_t *aad,
								   size_t aad_len, const uint8_t *pt,
								   size_t pt_len, uint8_t *ct, uint8_t tag[16])
{
	uint8_t H[16], J0[16], EK_J0[16], CTR[16];
	uint8_t X[16];
	uint64_t aad_bits, ct_bits;
	size_t i, k;

	for (i = 0; i < 16; i++)
		H[i] = 0;
	fun_crypto_aes128_encrypt(key, H, H); /* H = AES(K, 0^128) */

	/* J0 */
	if (iv_len == 12) {
		for (i = 0; i < 12; i++)
			J0[i] = iv[i];
		for (i = 0; i < 3; i++)
			J0[12 + i] = 0;
		J0[15] = 1;
	} else {
		for (i = 0; i < 16; i++)
			J0[i] = 0;
		for (i = 0; i < iv_len; i += 16) {
			size_t blk = (iv_len - i < 16) ? (iv_len - i) : 16;
			uint8_t tmp[16];
			for (k = 0; k < 16; k++)
				tmp[k] = 0;
			for (k = 0; k < blk; k++)
				tmp[k] = iv[i + k];
			xor16(tmp, J0, tmp);
			gf128_mul(tmp, H, J0);
		}
		uint8_t len_blk[16];
		for (i = 0; i < 16; i++)
			len_blk[i] = 0;
		aad_bits = (uint64_t)iv_len * 8;
		len_blk[8] = (uint8_t)(aad_bits >> 56);
		len_blk[9] = (uint8_t)(aad_bits >> 48);
		len_blk[10] = (uint8_t)(aad_bits >> 40);
		len_blk[11] = (uint8_t)(aad_bits >> 32);
		len_blk[12] = (uint8_t)(aad_bits >> 24);
		len_blk[13] = (uint8_t)(aad_bits >> 16);
		len_blk[14] = (uint8_t)(aad_bits >> 8);
		len_blk[15] = (uint8_t)aad_bits;
		xor16(len_blk, J0, len_blk);
		gf128_mul(len_blk, H, J0);
	}

	/* E(J0) */
	fun_crypto_aes128_encrypt(key, J0, EK_J0);

	/* CTR encrypt */
	for (i = 0; i < 16; i++)
		CTR[i] = J0[i];
	inc32(CTR);
	for (i = 0; i < pt_len; i++) {
		uint32_t block_off = i % 16;
		if (block_off == 0) {
			uint8_t enc[16];
			fun_crypto_aes128_encrypt(key, CTR, enc);
			inc32(CTR);
			uint32_t j;
			for (j = 0; j < 16 && i + j < pt_len; j++)
				ct[i + j] = (uint8_t)(pt[i + j] ^ enc[j]);
		}
	}

	/* GHASH */
	for (i = 0; i < 16; i++)
		X[i] = 0;
	for (i = 0; i < aad_len; i += 16) {
		size_t blk = (aad_len - i < 16) ? (aad_len - i) : 16;
		uint8_t tmp[16];
		for (k = 0; k < 16; k++)
			tmp[k] = 0;
		for (k = 0; k < blk; k++)
			tmp[k] = aad[i + k];
		xor16(tmp, X, tmp);
		gf128_mul(tmp, H, X);
	}
	for (i = 0; i < pt_len; i += 16) {
		size_t blk = (pt_len - i < 16) ? (pt_len - i) : 16;
		uint8_t tmp[16];
		for (k = 0; k < 16; k++)
			tmp[k] = 0;
		for (k = 0; k < blk; k++)
			tmp[k] = ct[i + k];
		xor16(tmp, X, tmp);
		gf128_mul(tmp, H, X);
	}

	uint8_t len_blk[16];
	for (i = 0; i < 16; i++)
		len_blk[i] = 0;
	aad_bits = (uint64_t)aad_len * 8;
	ct_bits = (uint64_t)pt_len * 8;
	len_blk[0] = (uint8_t)(aad_bits >> 56);
	len_blk[1] = (uint8_t)(aad_bits >> 48);
	len_blk[2] = (uint8_t)(aad_bits >> 40);
	len_blk[3] = (uint8_t)(aad_bits >> 32);
	len_blk[4] = (uint8_t)(aad_bits >> 24);
	len_blk[5] = (uint8_t)(aad_bits >> 16);
	len_blk[6] = (uint8_t)(aad_bits >> 8);
	len_blk[7] = (uint8_t)aad_bits;
	len_blk[8] = (uint8_t)(ct_bits >> 56);
	len_blk[9] = (uint8_t)(ct_bits >> 48);
	len_blk[10] = (uint8_t)(ct_bits >> 40);
	len_blk[11] = (uint8_t)(ct_bits >> 32);
	len_blk[12] = (uint8_t)(ct_bits >> 24);
	len_blk[13] = (uint8_t)(ct_bits >> 16);
	len_blk[14] = (uint8_t)(ct_bits >> 8);
	len_blk[15] = (uint8_t)ct_bits;
	xor16(len_blk, X, len_blk);
	gf128_mul(len_blk, H, X);

	xor16(tag, X, EK_J0);

	/* Scrub key-derived working state from the stack. */
	gcm_secure_zero(H, sizeof(H));
	gcm_secure_zero(J0, sizeof(J0));
	gcm_secure_zero(EK_J0, sizeof(EK_J0));
	gcm_secure_zero(CTR, sizeof(CTR));
	gcm_secure_zero(X, sizeof(X));
}

int fun_crypto_aes128_gcm_decrypt(const uint8_t key[16], const uint8_t *iv,
								  size_t iv_len, const uint8_t *aad,
								  size_t aad_len, const uint8_t *ct,
								  size_t ct_len, const uint8_t tag[16],
								  uint8_t *pt)
{
	uint8_t H[16], J0[16], EK_J0[16], CTR[16];
	uint8_t X[16], computed_tag[16];
	uint64_t aad_bits, ct_bits;
	size_t i, k;
	uint32_t j;

	for (i = 0; i < 16; i++)
		H[i] = 0;
	fun_crypto_aes128_encrypt(key, H, H);

	if (iv_len == 12) {
		for (i = 0; i < 12; i++)
			J0[i] = iv[i];
		for (i = 0; i < 3; i++)
			J0[12 + i] = 0;
		J0[15] = 1;
	} else {
		for (i = 0; i < 16; i++)
			J0[i] = 0;
		for (i = 0; i < iv_len; i += 16) {
			size_t blk = (iv_len - i < 16) ? (iv_len - i) : 16;
			uint8_t tmp[16];
			for (k = 0; k < 16; k++)
				tmp[k] = 0;
			for (k = 0; k < blk; k++)
				tmp[k] = iv[i + k];
			xor16(tmp, J0, tmp);
			gf128_mul(tmp, H, J0);
		}
		uint8_t len_blk[16];
		for (i = 0; i < 16; i++)
			len_blk[i] = 0;
		aad_bits = (uint64_t)iv_len * 8;
		len_blk[8] = (uint8_t)(aad_bits >> 56);
		len_blk[9] = (uint8_t)(aad_bits >> 48);
		len_blk[10] = (uint8_t)(aad_bits >> 40);
		len_blk[11] = (uint8_t)(aad_bits >> 32);
		len_blk[12] = (uint8_t)(aad_bits >> 24);
		len_blk[13] = (uint8_t)(aad_bits >> 16);
		len_blk[14] = (uint8_t)(aad_bits >> 8);
		len_blk[15] = (uint8_t)aad_bits;
		xor16(len_blk, J0, len_blk);
		gf128_mul(len_blk, H, J0);
	}

	fun_crypto_aes128_encrypt(key, J0, EK_J0);

	/* Verify tag over AAD || ciphertext || lengths *before* decrypting.
	   This keeps the ciphertext intact for GHASH even when pt aliases ct
	   (in-place decryption), and verifies before releasing plaintext. */
	for (i = 0; i < 16; i++)
		X[i] = 0;
	for (i = 0; i < aad_len; i += 16) {
		size_t blk = (aad_len - i < 16) ? (aad_len - i) : 16;
		uint8_t tmp[16];
		for (k = 0; k < 16; k++)
			tmp[k] = 0;
		for (k = 0; k < blk; k++)
			tmp[k] = aad[i + k];
		xor16(tmp, X, tmp);
		gf128_mul(tmp, H, X);
	}
	for (i = 0; i < ct_len; i += 16) {
		size_t blk = (ct_len - i < 16) ? (ct_len - i) : 16;
		uint8_t tmp[16];
		for (k = 0; k < 16; k++)
			tmp[k] = 0;
		for (k = 0; k < blk; k++)
			tmp[k] = ct[i + k];
		xor16(tmp, X, tmp);
		gf128_mul(tmp, H, X);
	}
	{
		uint8_t len_blk[16];
		for (i = 0; i < 16; i++)
			len_blk[i] = 0;
		aad_bits = (uint64_t)aad_len * 8;
		ct_bits = (uint64_t)ct_len * 8;
		len_blk[0] = (uint8_t)(aad_bits >> 56);
		len_blk[1] = (uint8_t)(aad_bits >> 48);
		len_blk[2] = (uint8_t)(aad_bits >> 40);
		len_blk[3] = (uint8_t)(aad_bits >> 32);
		len_blk[4] = (uint8_t)(aad_bits >> 24);
		len_blk[5] = (uint8_t)(aad_bits >> 16);
		len_blk[6] = (uint8_t)(aad_bits >> 8);
		len_blk[7] = (uint8_t)aad_bits;
		len_blk[8] = (uint8_t)(ct_bits >> 56);
		len_blk[9] = (uint8_t)(ct_bits >> 48);
		len_blk[10] = (uint8_t)(ct_bits >> 40);
		len_blk[11] = (uint8_t)(ct_bits >> 32);
		len_blk[12] = (uint8_t)(ct_bits >> 24);
		len_blk[13] = (uint8_t)(ct_bits >> 16);
		len_blk[14] = (uint8_t)(ct_bits >> 8);
		len_blk[15] = (uint8_t)ct_bits;
		xor16(len_blk, X, len_blk);
		gf128_mul(len_blk, H, X);
	}
	xor16(computed_tag, X, EK_J0);

	/* Constant-time tag comparison: accumulate XOR of all differences */
	{
		uint8_t diff = 0;
		for (i = 0; i < 16; i++)
			diff |= (uint8_t)(computed_tag[i] ^ tag[i]);
		if (diff != 0) {
			for (i = 0; i < ct_len; i++)
				pt[i] = 0;
			gcm_secure_zero(H, sizeof(H));
			gcm_secure_zero(J0, sizeof(J0));
			gcm_secure_zero(EK_J0, sizeof(EK_J0));
			gcm_secure_zero(CTR, sizeof(CTR));
			gcm_secure_zero(X, sizeof(X));
			gcm_secure_zero(computed_tag, sizeof(computed_tag));
			return -1;
		}
	}

	/* Authentication succeeded: CTR-decrypt ciphertext into plaintext.
	   Safe for in-place operation (pt == ct) since GHASH is complete. */
	for (i = 0; i < 16; i++)
		CTR[i] = J0[i];
	inc32(CTR);
	for (i = 0; i < ct_len; i++) {
		uint32_t block_off = (uint32_t)(i % 16);
		if (block_off == 0) {
			uint8_t enc[16];
			fun_crypto_aes128_encrypt(key, CTR, enc);
			inc32(CTR);
			for (j = 0; j < 16 && i + j < ct_len; j++)
				pt[i + j] = (uint8_t)(ct[i + j] ^ enc[j]);
		}
	}
	gcm_secure_zero(H, sizeof(H));
	gcm_secure_zero(J0, sizeof(J0));
	gcm_secure_zero(EK_J0, sizeof(EK_J0));
	gcm_secure_zero(CTR, sizeof(CTR));
	gcm_secure_zero(X, sizeof(X));
	gcm_secure_zero(computed_tag, sizeof(computed_tag));
	return 0;
}