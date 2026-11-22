#ifndef LIBRARY_CRYPTO_H
#define LIBRARY_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

/*
 * Crypto module — cryptographic primitives for TLS.
 *
 * All functions operate on caller-provided buffers. No heap allocations.
 * No C standard library dependency. Pure computation.
 *
 * Primitives:
 *   - SHA-256 hash
 *   - HMAC-SHA256
 *   - AES-128 block encrypt
 *   - AES-128-GCM authenticated encrypt/decrypt
 *   - ECDH key exchange on P-256
 *   - ECDSA signature verification on P-256
 *   - X.509 certificate parsing and chain validation
 *   - PEM parsing
 */

/* ------------------------------------------------------------------
 * SHA-256
 * ------------------------------------------------------------------ */

/*
 * Compute SHA-256 hash of input.
 * output must point to a 32-byte buffer.
 */
void fun_crypto_sha256(const uint8_t *input, size_t length, uint8_t output[32]);

/* ------------------------------------------------------------------
 * HMAC-SHA256
 * ------------------------------------------------------------------ */

/*
 * Compute HMAC-SHA256.
 * key/msg may be any length up to 4096 bytes. output must point to a 32-byte buffer.
 * Returns 0 on success, -1 if the message exceeds the buffer limit.
 */
int fun_crypto_hmac_sha256(const uint8_t *key, size_t key_len,
						   const uint8_t *msg, size_t msg_len,
						   uint8_t output[32]);

/* ------------------------------------------------------------------
 * AES-128 block cipher
 * ------------------------------------------------------------------ */

/*
 * Encrypt a single 16-byte block with AES-128.
 * key must point to 16 bytes. in/out must point to 16 bytes.
 */
void fun_crypto_aes128_encrypt(const uint8_t key[16], const uint8_t in[16],
							   uint8_t out[16]);

/* ------------------------------------------------------------------
 * AES-128-GCM authenticated encryption
 * ------------------------------------------------------------------ */

/*
 * Encrypt plaintext with AES-128-GCM.
 * key: 16 bytes. iv: typically 12 bytes.
 * aad: additional authenticated data (may be NULL if aad_len == 0).
 * plaintext/ciphertext: caller-allocated, pt_len bytes.
 * tag: caller-allocated, 16 bytes.
 */
void fun_crypto_aes128_gcm_encrypt(const uint8_t key[16], const uint8_t *iv,
								   size_t iv_len, const uint8_t *aad,
								   size_t aad_len, const uint8_t *plaintext,
								   size_t pt_len, uint8_t *ciphertext,
								   uint8_t tag[16]);

/*
 * Decrypt and verify AES-128-GCM ciphertext.
 * Returns 0 on success (tag verified), non-zero on authentication failure.
 */
int fun_crypto_aes128_gcm_decrypt(const uint8_t key[16], const uint8_t *iv,
								  size_t iv_len, const uint8_t *aad,
								  size_t aad_len, const uint8_t *ciphertext,
								  size_t ct_len, const uint8_t tag[16],
								  uint8_t *plaintext);

/* ------------------------------------------------------------------
 * ECDH on NIST P-256
 * ------------------------------------------------------------------ */

/*
 * EC point on secp256r1.
 * Each coordinate is 32 bytes, big-endian.
 */
typedef struct {
	uint8_t x[32];
	uint8_t y[32];
} fun_crypto_ec_point;

/*
 * Generate a public key from a private key on P-256.
 * private_key: 32 bytes (scalar). public_key: output (x || y, 32 bytes each).
 * Returns 0 on success. Returns -1 if the private key is zero or >= the
 * group order n; in that case public_key is set to the all-zero point.
 */
int fun_crypto_ec_p256_public_key(const uint8_t private_key[32],
								  fun_crypto_ec_point *public_key);

/*
 * Compute ECDH shared secret on P-256.
 * private_key: 32 bytes. peer_public: peer's public key.
 * shared_secret: output, 32 bytes (x-coordinate of result).
 * Returns 0 on success. Returns -1 if the private key is zero or >= the
 * group order, or if the peer public key is invalid (not on the curve,
 * at infinity, or a coordinate >= p). On failure shared_secret is zeroed
 * and the caller MUST abort the handshake — do not derive keys from it.
 */
int fun_crypto_ecdh_p256(const uint8_t private_key[32],
						 const fun_crypto_ec_point *peer_public,
						 uint8_t shared_secret[32]);

/* ------------------------------------------------------------------
 * ECDSA on NIST P-256
 * ------------------------------------------------------------------ */

/*
 * Verify an ECDSA signature on P-256.
 * hash: 32-byte message hash.
 * public_key: signer's public key (must be a valid point on the curve).
 * sig_r, sig_s: signature components (32 bytes each, big-endian); both
 *   must be in [1, n) where n is the group order.
 * Returns 0 if signature is valid, non-zero if invalid.
 *
 * Note on malleability: ECDSA signatures are malleable — (r, s) and
 * (r, n-s) are both valid for the same message. This function accepts
 * both encodings. TLS (RFC 5246/8446) does not require a canonical
 * low-S encoding, and peers may send either, so rejection of high-S
 * here would break interoperability. Callers that require a unique
 * signature encoding (e.g. anti-replay identifiers) must normalize to
 * low-S themselves (replace s with n-s when s > n/2).
 */
int fun_crypto_ecdsa_verify(const uint8_t hash[32],
							const fun_crypto_ec_point *public_key,
							const uint8_t sig_r[32], const uint8_t sig_s[32]);

/* ------------------------------------------------------------------
 * X.509 certificate parsing
 * ------------------------------------------------------------------ */

typedef struct {
	const uint8_t *der;
	size_t der_len;
} fun_crypto_x509_cert;

/*
 * Parse a DER-encoded X.509 certificate.
 * Returns 0 on success, non-zero on parse error.
 */
int fun_crypto_x509_parse(const uint8_t *der, size_t der_len,
						  fun_crypto_x509_cert *cert);

/*
 * Validate a certificate chain against a set of trusted roots.
 * certs: array of certificates (server cert first, then intermediates).
 * num_certs: number of certificates in the chain.
 * trusted_roots: array of trusted root certificates.
 * num_roots: number of trusted roots.
 * hostname: expected hostname for SAN/CN matching.
 * Returns 0 on success, non-zero error code on failure.
 */
int fun_crypto_x509_validate_chain(const fun_crypto_x509_cert *certs,
								   size_t num_certs,
								   const fun_crypto_x509_cert *trusted_roots,
								   size_t num_roots, const char *hostname);

/* ------------------------------------------------------------------
 * PEM parsing
 * ------------------------------------------------------------------ */

/*
 * Parse a PEM file containing X.509 certificates.
 * pem_data: raw PEM file content.
 * pem_len: length of PEM data.
 * der_outputs: caller-provided buffer for DER certificate data.
 * der_sizes: caller-provided buffer for DER certificate sizes.
 * max_certs: maximum number of certificates to extract (size of der_outputs array).
 * Returns the number of certificates extracted, or -1 on parse error.
 */
int fun_crypto_pem_parse(const char *pem_data, size_t pem_len,
						 uint8_t *der_outputs[], size_t der_sizes[],
						 size_t *num_certs, size_t max_certs);

#endif /* LIBRARY_CRYPTO_H */