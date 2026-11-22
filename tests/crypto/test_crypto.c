/* crypto module tests — SHA-256, HMAC, AES-128, AES-128-GCM, EC P-256, ECDH, ECDSA */
#include "fundamental/bignumber/bignumber.h"
#include "fundamental/console/console.h"
#include "fundamental/crypto/crypto.h"
#include "fundamental/error/error.h"

#include "test_certs.h"

#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

static int g_failures;

static void expect(int cond, const char *name)
{
	if (cond) {
		fun_console_write(GREEN_CHECK);
		fun_console_write(" ");
		fun_console_write_line(name);
	} else {
		fun_console_write("FAIL: ");
		fun_console_write_line(name);
		g_failures++;
	}
}

static int bytes_eq(const uint8_t *a, const uint8_t *b, unsigned n)
{
	unsigned i;
	for (i = 0; i < n; i++)
		if (a[i] != b[i])
			return 0;
	return 1;
}

static void fill(uint8_t *p, unsigned n, uint8_t v)
{
	unsigned i;
	for (i = 0; i < n; i++)
		p[i] = v;
}

int main(void)
{
	/* ---- SHA-256 ---- */
	{
		static const uint8_t exp[32] = {
			0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
			0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
			0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
		};
		uint8_t out[32];
		fun_crypto_sha256(NULL, 0, out);
		expect(bytes_eq(out, exp, 32), "test_sha256_empty");
	}
	{
		static const uint8_t exp[32] = {
			0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40,
			0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17,
			0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
		};
		const char *msg = "abc";
		uint8_t out[32];
		fun_crypto_sha256((const uint8_t *)msg, 3, out);
		expect(bytes_eq(out, exp, 32), "test_sha256_abc");
	}
	{
		static const uint8_t exp[32] = {
			0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26,
			0x93, 0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff,
			0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
		};
		const char *msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmn"
						  "lmnomnopnopq";
		uint8_t out[32];
		fun_crypto_sha256((const uint8_t *)msg, 56, out);
		expect(bytes_eq(out, exp, 32), "test_sha256_448bit");
	}

	/* ---- HMAC-SHA256 (RFC 4231) ---- */
	{
		static const uint8_t key[20] = { 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
										 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
										 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
										 0x0b, 0x0b, 0x0b, 0x0b, 0x0b };
		const char *msg = "Hi There";
		static const uint8_t exp[32] = {
			0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf,
			0xce, 0xaf, 0x0b, 0xf1, 0x2b, 0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83,
			0x3d, 0xa7, 0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7
		};
		uint8_t out[32];
		int r = fun_crypto_hmac_sha256(key, 20, (const uint8_t *)msg, 8, out);
		expect(r == 0 && bytes_eq(out, exp, 32), "test_hmac_rfc4231_1");
	}
	{
		const char *key = "Jefe";
		const char *msg = "what do ya want for nothing?";
		static const uint8_t exp[32] = {
			0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e, 0x6a, 0x04, 0x24,
			0x26, 0x08, 0x95, 0x75, 0xc7, 0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27,
			0x39, 0x83, 0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43
		};
		uint8_t out[32];
		int r = fun_crypto_hmac_sha256((const uint8_t *)key, 4,
									   (const uint8_t *)msg, 28, out);
		expect(r == 0 && bytes_eq(out, exp, 32), "test_hmac_rfc4231_2");
	}
	{
		/* RFC 4231 Test Case 3: key > 64 bytes (exercises hashing path) */
		uint8_t key[131];
		const char *msg =
			"Test Using Larger Than Block-Size Key - Hash Key First";
		static const uint8_t exp[32] = {
			0x60, 0xe4, 0x31, 0x59, 0x1e, 0xe0, 0xb6, 0x7f, 0x0d, 0x8a, 0x26,
			0xaa, 0xcb, 0xf5, 0xb7, 0x7f, 0x8e, 0x0b, 0xc6, 0x21, 0x37, 0x28,
			0xc5, 0x14, 0x05, 0x46, 0x04, 0x0f, 0x0e, 0xe3, 0x7f, 0x54
		};
		uint8_t out[32];
		unsigned i;
		for (i = 0; i < sizeof(key); i++)
			key[i] = 0xaa;
		int r = fun_crypto_hmac_sha256(key, sizeof(key), (const uint8_t *)msg,
									   54, out);
		expect(r == 0 && bytes_eq(out, exp, 32), "test_hmac_rfc4231_3");
	}

	/* ---- AES-128 ---- */
	{
		static const uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae,
										 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88,
										 0x09, 0xcf, 0x4f, 0x3c };
		static const uint8_t pt[16] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a,
										0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2,
										0xe0, 0x37, 0x07, 0x34 };
		static const uint8_t exp[16] = { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc,
										 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97,
										 0x19, 0x6a, 0x0b, 0x32 };
		uint8_t ct[16];
		fun_crypto_aes128_encrypt(key, pt, ct);
		expect(bytes_eq(ct, exp, 16), "test_aes128_fips197");
	}
	{
		uint8_t key[16], pt[16], ct[16];
		static const uint8_t exp[16] = { 0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a,
										 0x2c, 0x3b, 0x88, 0x4c, 0xfa, 0x59,
										 0xca, 0x34, 0x2b, 0x2e };
		fill(key, 16, 0);
		fill(pt, 16, 0);
		fun_crypto_aes128_encrypt(key, pt, ct);
		expect(bytes_eq(ct, exp, 16), "test_aes128_zero");
	}

	/* ---- AES-128-GCM ---- */
	{
		uint8_t key[16], iv[12], tag[16];
		static const uint8_t exp[16] = { 0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e,
										 0x30, 0x61, 0x36, 0x7f, 0x1d, 0x57,
										 0xa4, 0xe7, 0x45, 0x5a };
		fill(key, 16, 0);
		fill(iv, 12, 0);
		fun_crypto_aes128_gcm_encrypt(key, iv, 12, NULL, 0, NULL, 0, NULL, tag);
		expect(bytes_eq(tag, exp, 16), "test_gcm_empty");
	}
	{
		uint8_t key[16], iv[12], ct[16], tag[16], rec[16];
		uint8_t ptx[16];
		static const uint8_t exp_ct[16] = { 0x03, 0x88, 0xda, 0xce, 0x60, 0xb6,
											0xa3, 0x92, 0xf3, 0x28, 0xc2, 0xb9,
											0x71, 0xb2, 0xfe, 0x78 };
		static const uint8_t exp_tag[16] = { 0xab, 0x6e, 0x47, 0xd4, 0x2c, 0xec,
											 0x13, 0xbd, 0xf5, 0x3a, 0x67, 0xb2,
											 0x12, 0x57, 0xbd, 0xdf };
		fill(key, 16, 0);
		fill(iv, 12, 0);
		fill(ptx, 16, 0);
		fun_crypto_aes128_gcm_encrypt(key, iv, 12, NULL, 0, ptx, 16, ct, tag);
		expect(bytes_eq(ct, exp_ct, 16) && bytes_eq(tag, exp_tag, 16),
			   "test_gcm_16b");

		expect(fun_crypto_aes128_gcm_decrypt(key, iv, 12, NULL, 0, ct, 16, tag,
											 rec) == 0 &&
				   bytes_eq(rec, ptx, 16),
			   "test_gcm_roundtrip");
	}
	{
		/* 64-byte plaintext + AAD round-trip */
		uint8_t key[16], iv[12], pt[64], ct[64], rec[64], aad[20];
		uint8_t tag[16];
		unsigned i;
		for (i = 0; i < 16; i++)
			key[i] = (uint8_t)(i + 1);
		for (i = 0; i < 12; i++)
			iv[i] = (uint8_t)(0xA0 + i);
		for (i = 0; i < 64; i++)
			pt[i] = (uint8_t)(i * 3 + 7);
		for (i = 0; i < 20; i++)
			aad[i] = (uint8_t)(0xC0 + i);
		fun_crypto_aes128_gcm_encrypt(key, iv, 12, aad, 20, pt, 64, ct, tag);
		int ok = fun_crypto_aes128_gcm_decrypt(key, iv, 12, aad, 20, ct, 64,
											   tag, rec) == 0 &&
				 bytes_eq(rec, pt, 64);
		expect(ok, "test_gcm_64b_aad");
	}
	{
		/* Bad tag: decrypt must fail and zero the plaintext */
		uint8_t key[16], iv[12], pt[16], ct[16], tag[16], bad[16];
		uint8_t rec[16];
		fill(key, 16, 0);
		fill(iv, 12, 0);
		fill(pt, 16, 0x5A);
		fun_crypto_aes128_gcm_encrypt(key, iv, 12, NULL, 0, pt, 16, ct, tag);
		fill(bad, 16, 0);
		fill(rec, 16, 0xAA);
		int r = fun_crypto_aes128_gcm_decrypt(key, iv, 12, NULL, 0, ct, 16, bad,
											  rec);
		int zeroed = 1;
		unsigned i;
		for (i = 0; i < 16; i++)
			if (rec[i] != 0)
				zeroed = 0;
		expect(r != 0 && zeroed, "test_gcm_bad_tag_zeroes_plaintext");
	}
	{
		/* In-place decryption (pt aliases ct) must still verify and
		   recover the plaintext (GHASH runs over ciphertext first). */
		uint8_t key[16], iv[12], pt[32], buf[32], tag[16], orig[32];
		unsigned i;
		for (i = 0; i < 16; i++)
			key[i] = (uint8_t)(0x30 + i);
		for (i = 0; i < 12; i++)
			iv[i] = (uint8_t)(0x80 + i);
		for (i = 0; i < 32; i++) {
			pt[i] = (uint8_t)(i * 5 + 1);
			orig[i] = pt[i];
		}
		fun_crypto_aes128_gcm_encrypt(key, iv, 12, NULL, 0, pt, 32, buf, tag);
		int r = fun_crypto_aes128_gcm_decrypt(key, iv, 12, NULL, 0, buf, 32,
											  tag, buf);
		expect(r == 0 && bytes_eq(buf, orig, 32), "test_gcm_in_place");
	}
	{
		/* Non-96-bit IV (exercises the GHASH-derived J0 path) */
		uint8_t key[16], iv[8], pt[20], ct[20], rec[20], tag[16];
		unsigned i;
		for (i = 0; i < 16; i++)
			key[i] = (uint8_t)(0x11 * (i + 1));
		for (i = 0; i < 8; i++)
			iv[i] = (uint8_t)(0x70 + i);
		for (i = 0; i < 20; i++)
			pt[i] = (uint8_t)(i * 7 + 3);
		fun_crypto_aes128_gcm_encrypt(key, iv, 8, NULL, 0, pt, 20, ct, tag);
		int r = fun_crypto_aes128_gcm_decrypt(key, iv, 8, NULL, 0, ct, 20, tag,
											  rec);
		expect(r == 0 && bytes_eq(rec, pt, 20), "test_gcm_non12_iv");
	}

	/* ---- EC P-256: 2*G ---- */
	{
		static const uint8_t exp_x[32] = {
			0x7c, 0xf2, 0x7b, 0x18, 0x8d, 0x03, 0x4f, 0x7e, 0x8a, 0x52, 0x38,
			0x03, 0x04, 0xb5, 0x1a, 0xc3, 0xc0, 0x89, 0x69, 0xe2, 0x77, 0xf2,
			0x1b, 0x35, 0xa6, 0x0b, 0x48, 0xfc, 0x47, 0x66, 0x99, 0x78
		};
		static const uint8_t exp_y[32] = {
			0x07, 0x77, 0x55, 0x10, 0xdb, 0x8e, 0xd0, 0x40, 0x29, 0x3d, 0x9a,
			0xc6, 0x9f, 0x74, 0x30, 0xdb, 0xba, 0x7d, 0xad, 0xe6, 0x3c, 0xe9,
			0x82, 0x29, 0x9e, 0x04, 0xb7, 0x9d, 0x22, 0x78, 0x73, 0xd1
		};
		uint8_t k2[32];
		fun_crypto_ec_point pub;
		fill(k2, 32, 0);
		k2[31] = 2;
		fun_crypto_ec_p256_public_key(k2, &pub);
		expect(bytes_eq(pub.x, exp_x, 32) && bytes_eq(pub.y, exp_y, 32),
			   "test_ec_p256_2G");
	}
	{
		/* k = 1 must yield the generator G itself */
		static const uint8_t Gx[32] = {
			0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47, 0xf8, 0xbc, 0xe6,
			0xe5, 0x63, 0xa4, 0x40, 0xf2, 0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb,
			0x33, 0xa0, 0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96
		};
		static const uint8_t Gy[32] = {
			0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b, 0x8e, 0xe7, 0xeb,
			0x4a, 0x7c, 0x0f, 0x9e, 0x16, 0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31,
			0x5e, 0xce, 0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5
		};
		uint8_t k1[32];
		fun_crypto_ec_point pub;
		fill(k1, 32, 0);
		k1[31] = 1;
		int r = fun_crypto_ec_p256_public_key(k1, &pub);
		expect(r == 0 && bytes_eq(pub.x, Gx, 32) && bytes_eq(pub.y, Gy, 32),
			   "test_ec_p256_1G");
	}

	/* ---- ECDH ---- */
	{
		/* ecdh(dA=2, peer=G) must equal 2*G (x-coordinate).

		   This is an absolute check derived from the trusted 2*G vector
		   above: 2*G is independently verified via public_key(2). */
		static const uint8_t Gx[32] = {
			0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47, 0xf8, 0xbc, 0xe6,
			0xe5, 0x63, 0xa4, 0x40, 0xf2, 0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb,
			0x33, 0xa0, 0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96
		};
		static const uint8_t Gy[32] = {
			0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b, 0x8e, 0xe7, 0xeb,
			0x4a, 0x7c, 0x0f, 0x9e, 0x16, 0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31,
			0x5e, 0xce, 0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5
		};
		static const uint8_t exp[32] = {
			0x7c, 0xf2, 0x7b, 0x18, 0x8d, 0x03, 0x4f, 0x7e, 0x8a, 0x52, 0x38,
			0x03, 0x04, 0xb5, 0x1a, 0xc3, 0xc0, 0x89, 0x69, 0xe2, 0x77, 0xf2,
			0x1b, 0x35, 0xa6, 0x0b, 0x48, 0xfc, 0x47, 0x66, 0x99, 0x78
		};
		fun_crypto_ec_point G;
		uint8_t dA[32], secret[32];
		unsigned i;
		for (i = 0; i < 32; i++) {
			G.x[i] = Gx[i];
			G.y[i] = Gy[i];
		}
		fill(dA, 32, 0);
		dA[31] = 2;
		int r = fun_crypto_ecdh_p256(dA, &G, secret);
		expect(r == 0 && bytes_eq(secret, exp, 32), "test_ecdh_p256");
	}
	{
		/* Invalid peer point (0,0) must be rejected and zero the output */
		fun_crypto_ec_point bad;
		uint8_t dA[32], secret[32];
		unsigned i;
		int zeroed = 1;
		fill(bad.x, 32, 0);
		fill(bad.y, 32, 0);
		fill(dA, 32, 0);
		dA[31] = 2;
		fill(secret, 32, 0xAA);
		int r = fun_crypto_ecdh_p256(dA, &bad, secret);
		for (i = 0; i < 32; i++)
			if (secret[i] != 0)
				zeroed = 0;
		expect(r != 0 && zeroed, "test_ecdh_invalid_peer");
	}

	/* ---- ECDSA (RFC 6979 A.2.5, message "sample") ---- */
	{
		static const uint8_t hash[32] = {
			0xaf, 0x2b, 0xdb, 0xe1, 0xaa, 0x9b, 0x6e, 0xc1, 0xe2, 0xad, 0xe1,
			0xd6, 0x94, 0xf4, 0x1f, 0xc7, 0x1a, 0x83, 0x1d, 0x02, 0x68, 0xe9,
			0x89, 0x15, 0x62, 0x11, 0x3d, 0x8a, 0x62, 0xad, 0xd1, 0xbf
		};
		static const uint8_t Qx[32] = {
			0x60, 0xfe, 0xd4, 0xba, 0x25, 0x5a, 0x9d, 0x31, 0xc9, 0x61, 0xeb,
			0x74, 0xc6, 0x35, 0x6d, 0x68, 0xc0, 0x49, 0xb8, 0x92, 0x3b, 0x61,
			0xfa, 0x6c, 0xe6, 0x69, 0x62, 0x2e, 0x60, 0xf2, 0x9f, 0xb6
		};
		static const uint8_t Qy[32] = {
			0x79, 0x03, 0xfe, 0x10, 0x08, 0xb8, 0xbc, 0x99, 0xa4, 0x1a, 0xe9,
			0xe9, 0x56, 0x28, 0xbc, 0x64, 0xf2, 0xf1, 0xb2, 0x0c, 0x2d, 0x7e,
			0x9f, 0x51, 0x77, 0xa3, 0xc2, 0x94, 0xd4, 0x46, 0x22, 0x99
		};
		static const uint8_t r[32] = { 0xef, 0xd4, 0x8b, 0x2a, 0xac, 0xb6, 0xa8,
									   0xfd, 0x11, 0x40, 0xdd, 0x9c, 0xd4, 0x5e,
									   0x81, 0xd6, 0x9d, 0x2c, 0x87, 0x7b, 0x56,
									   0xaa, 0xf9, 0x91, 0xc3, 0x4d, 0x0e, 0xa8,
									   0x4e, 0xaf, 0x37, 0x16 };
		static const uint8_t s[32] = { 0xf7, 0xcb, 0x1c, 0x94, 0x2d, 0x65, 0x7c,
									   0x41, 0xd4, 0x36, 0xc7, 0xa1, 0xb6, 0xe2,
									   0x9f, 0x65, 0xf3, 0xe9, 0x00, 0xdb, 0xb9,
									   0xaf, 0xf4, 0x06, 0x4d, 0xc4, 0xab, 0x2f,
									   0x84, 0x3a, 0xcd, 0xa8 };
		fun_crypto_ec_point Q;
		unsigned i;
		for (i = 0; i < 32; i++) {
			Q.x[i] = Qx[i];
			Q.y[i] = Qy[i];
		}
		expect(fun_crypto_ecdsa_verify(hash, &Q, r, s) == 0,
			   "test_ecdsa_valid");

		/* Tampered signature must be rejected */
		{
			uint8_t bad_s[32];
			for (i = 0; i < 32; i++)
				bad_s[i] = s[i];
			bad_s[31] ^= 0x01;
			expect(fun_crypto_ecdsa_verify(hash, &Q, r, bad_s) != 0,
				   "test_ecdsa_tampered");
		}

		/* Malleability: (r, n-s) is also a valid encoding and must
		   verify (TLS requires accepting both). */
		{
			static const uint8_t n_bytes[32] = {
				0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
				0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
			};
			BigNumber n = fun_bignumber_from_bytes(8, n_bytes);
			BigNumber sv = fun_bignumber_from_bytes(8, s);
			BigNumber ns = fun_bignumber_subtract(&n, &sv);
			uint8_t alt_s[32];
			fun_bignumber_to_bytes(&ns, alt_s);
			expect(fun_crypto_ecdsa_verify(hash, &Q, r, alt_s) == 0,
				   "test_ecdsa_high_s_malleability");
		}
	}

	/* ---- PEM parsing ---- */
	{
		uint8_t buf[1024];
		uint8_t *outs[1] = { buf };
		size_t sizes[1] = { sizeof(buf) };
		size_t count = 0;
		int r = fun_crypto_pem_parse(test_leaf_pem, sizeof(test_leaf_pem) - 1,
									 outs, sizes, &count, 1);
		expect(r == 1 && count == 1 && sizes[0] == sizeof(test_leaf_der) &&
				   bytes_eq(buf, test_leaf_der, sizeof(test_leaf_der)),
			   "test_pem_parse_single");
	}
	{
		uint8_t b0[1024], b1[1024], b2[1024];
		uint8_t *outs[3] = { b0, b1, b2 };
		size_t sizes[3] = { sizeof(b0), sizeof(b1), sizeof(b2) };
		size_t count = 0;
		int r = fun_crypto_pem_parse(test_bundle_pem,
									 sizeof(test_bundle_pem) - 1, outs, sizes,
									 &count, 3);
		expect(r == 3 && count == 3 && sizes[0] == sizeof(test_root_der) &&
				   sizes[1] == sizeof(test_intermediate_der) &&
				   sizes[2] == sizeof(test_leaf_der) &&
				   bytes_eq(b0, test_root_der, sizeof(test_root_der)) &&
				   bytes_eq(b1, test_intermediate_der,
							sizeof(test_intermediate_der)) &&
				   bytes_eq(b2, test_leaf_der, sizeof(test_leaf_der)),
			   "test_pem_parse_bundle");
	}
	{
		static const char bad[] = "-----BEGIN CERTIFICATE-----\nAAAA\n";
		uint8_t buf[64];
		uint8_t *outs[1] = { buf };
		size_t sizes[1] = { sizeof(buf) };
		size_t count = 0;
		int r =
			fun_crypto_pem_parse(bad, sizeof(bad) - 1, outs, sizes, &count, 1);
		expect(r == -1, "test_pem_reject_missing_end");
	}
	{
		static const char bad[] = "-----BEGIN CERTIFICATE-----\n!!!!\n"
								  "-----END CERTIFICATE-----\n";
		uint8_t buf[64];
		uint8_t *outs[1] = { buf };
		size_t sizes[1] = { sizeof(buf) };
		size_t count = 0;
		int r =
			fun_crypto_pem_parse(bad, sizeof(bad) - 1, outs, sizes, &count, 1);
		expect(r == -1, "test_pem_reject_bad_base64");
	}

	/* ---- X.509 parsing ---- */
	{
		fun_crypto_x509_cert leaf;
		int r =
			fun_crypto_x509_parse(test_leaf_der, sizeof(test_leaf_der), &leaf);
		expect(r == 0 && leaf.public_key_valid &&
				   leaf.not_after > leaf.not_before &&
				   leaf.common_name_len == 11 &&
				   bytes_eq((const uint8_t *)leaf.common_name,
							(const uint8_t *)"example.com", 11) &&
				   leaf.has_san && leaf.num_san_dns == 2 &&
				   leaf.san_dns_len[0] == 11 &&
				   bytes_eq((const uint8_t *)leaf.san_dns[0],
							(const uint8_t *)"example.com", 11) &&
				   bytes_eq((const uint8_t *)leaf.san_dns[1],
							(const uint8_t *)"www.example.com", 15) &&
				   leaf.has_basic_constraints && !leaf.is_ca &&
				   leaf.signature_algorithm ==
					   FUN_CRYPTO_X509_SIG_ECDSA_SHA256 &&
				   leaf.signature_len > 0,
			   "test_x509_parse_leaf");
	}
	{
		fun_crypto_x509_cert root;
		int r =
			fun_crypto_x509_parse(test_root_der, sizeof(test_root_der), &root);
		expect(r == 0 && root.public_key_valid && root.has_basic_constraints &&
				   root.is_ca && root.path_length == 1 && !root.has_san &&
				   root.common_name_len == 24 &&
				   bytes_eq((const uint8_t *)root.common_name,
							(const uint8_t *)"Fundamental Test Root CA", 24),
			   "test_x509_parse_root");
	}
	{
		fun_crypto_x509_cert leaf;
		int r = fun_crypto_x509_parse(test_leaf_der, sizeof(test_leaf_der) - 10,
									  &leaf);
		expect(r == ERROR_CODE_X509_PARSE_ERROR, "test_x509_reject_truncated");
	}

	/* ---- X.509 chain validation ---- */
	{
		fun_crypto_x509_cert leaf, intermediate, root, other_root;
		fun_crypto_x509_cert chain[2];
		fun_crypto_x509_cert roots[1];
		int ok = fun_crypto_x509_parse(test_leaf_der, sizeof(test_leaf_der),
									   &leaf) == 0 &&
				 fun_crypto_x509_parse(test_intermediate_der,
									   sizeof(test_intermediate_der),
									   &intermediate) == 0 &&
				 fun_crypto_x509_parse(test_root_der, sizeof(test_root_der),
									   &root) == 0 &&
				 fun_crypto_x509_parse(test_other_root_der,
									   sizeof(test_other_root_der),
									   &other_root) == 0;
		int64_t now = leaf.not_before + 86400;
		chain[0] = leaf;
		chain[1] = intermediate;
		roots[0] = root;

		expect(ok && fun_crypto_x509_validate_chain_at(chain, 2, roots, 1,
													   "example.com", now) == 0,
			   "test_x509_chain_valid");
		expect(ok && fun_crypto_x509_validate_chain_at(
						 chain, 2, roots, 1, "www.example.com", now) == 0,
			   "test_x509_chain_san_match");
		expect(ok && fun_crypto_x509_validate_chain_at(chain, 2, roots, 1,
													   "other.com", now) ==
						 ERROR_CODE_TLS_HOSTNAME_MISMATCH,
			   "test_x509_chain_hostname_mismatch");
		expect(ok && fun_crypto_x509_validate_chain_at(chain, 2, roots, 1,
													   "example.com",
													   leaf.not_after + 1) ==
						 ERROR_CODE_TLS_CERTIFICATE_EXPIRED,
			   "test_x509_chain_expired");
		expect(ok && fun_crypto_x509_validate_chain_at(chain, 2, roots, 1,
													   "example.com",
													   leaf.not_before - 1) ==
						 ERROR_CODE_TLS_CERTIFICATE_EXPIRED,
			   "test_x509_chain_not_yet_valid");
		expect(ok && fun_crypto_x509_validate_chain_at(chain, 2, &other_root, 1,
													   "example.com", now) ==
						 ERROR_CODE_TLS_CERTIFICATE_INVALID,
			   "test_x509_chain_wrong_root");
		expect(ok && fun_crypto_x509_validate_chain_at(chain, 1, roots, 1,
													   "example.com", now) ==
						 ERROR_CODE_TLS_CERTIFICATE_INVALID,
			   "test_x509_chain_missing_intermediate");

		/* CN fallback when the certificate has no SAN extension. */
		expect(ok && fun_crypto_x509_validate_chain_at(
						 &root, 1, &root, 1, "FUNDAMENTAL TEST ROOT CA", now) ==
						 0,
			   "test_x509_chain_cn_fallback");
	}
	{
		/* Flip a byte in the signature: the chain must be rejected. */
		fun_crypto_x509_cert leaf, intermediate, root;
		uint8_t tampered[sizeof(test_leaf_der)];
		fun_crypto_x509_cert chain[2];
		fun_crypto_x509_cert roots[1];
		size_t i;
		size_t off = 0;

		for (i = 0; i < sizeof(test_leaf_der); i++)
			tampered[i] = test_leaf_der[i];

		int ok = fun_crypto_x509_parse(test_leaf_der, sizeof(test_leaf_der),
									   &leaf) == 0 &&
				 fun_crypto_x509_parse(test_intermediate_der,
									   sizeof(test_intermediate_der),
									   &intermediate) == 0 &&
				 fun_crypto_x509_parse(test_root_der, sizeof(test_root_der),
									   &root) == 0;
		if (ok) {
			off = (size_t)(leaf.signature - leaf.der);
			tampered[off + leaf.signature_len - 1] ^= 0x01;
		}

		fun_crypto_x509_cert bad_leaf;
		int parsed =
			fun_crypto_x509_parse(tampered, sizeof(tampered), &bad_leaf);
		chain[0] = bad_leaf;
		chain[1] = intermediate;
		roots[0] = root;
		expect(ok && parsed == 0 &&
				   fun_crypto_x509_validate_chain_at(chain, 2, roots, 1, NULL,
													 leaf.not_before + 86400) ==
					   ERROR_CODE_TLS_CERTIFICATE_INVALID,
			   "test_x509_chain_tampered_signature");
	}

	fun_console_flush();
	return g_failures ? 1 : 0;
}