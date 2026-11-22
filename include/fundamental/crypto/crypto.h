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

#define FUN_CRYPTO_X509_MAX_SAN_DNS 16

#define FUN_CRYPTO_X509_SIG_UNKNOWN 0
#define FUN_CRYPTO_X509_SIG_ECDSA_SHA256 1

/*
 * Parsed view of a DER-encoded X.509 certificate.
 *
 * All pointers reference the caller's DER buffer; the structure does not
 * own or copy any data and is only valid while that buffer is alive.
 */
typedef struct {
	const uint8_t *der;
	size_t der_len;

	/* TBSCertificate, used as the signed message. */
	const uint8_t *tbs;
	size_t tbs_len;

	/* Full DER-encoded Name (RDNSequence) blobs. */
	const uint8_t *issuer;
	size_t issuer_len;
	const uint8_t *subject;
	size_t subject_len;

	/* Validity period, seconds since the Unix epoch (UTC). */
	int64_t not_before;
	int64_t not_after;

	/* Basic Constraints extension. */
	int has_basic_constraints;
	int is_ca;
	int path_length; /* -1 when absent */

	/* Subject Alternative Name dNSName entries (not NUL-terminated). */
	int has_san;
	const char *san_dns[FUN_CRYPTO_X509_MAX_SAN_DNS];
	size_t san_dns_len[FUN_CRYPTO_X509_MAX_SAN_DNS];
	size_t num_san_dns;

	/* Common names (OID 2.5.4.3), not NUL-terminated. */
	const char *common_name;
	size_t common_name_len;
	const char *issuer_common_name;
	size_t issuer_common_name_len;

	/* Subject public key. Only NIST P-256 EC keys are supported. */
	fun_crypto_ec_point public_key;
	int public_key_valid;

	/* Signature algorithm and raw signature (BIT STRING contents). */
	int signature_algorithm;
	const uint8_t *signature;
	size_t signature_len;
} fun_crypto_x509_cert;

/*
 * Parse a DER-encoded X.509 certificate.
 * Returns 0 on success, ERROR_CODE_X509_PARSE_ERROR on malformed input.
 * Certificates using an unsupported public key or signature algorithm are
 * rejected as well (only P-256 EC keys and ecdsa-with-SHA256 are supported).
 */
int fun_crypto_x509_parse(const uint8_t *der, size_t der_len,
						  fun_crypto_x509_cert *cert);

/*
 * Validate a certificate chain against a set of trusted roots.
 * certs: array of certificates (server cert first, then intermediates).
 * num_certs: number of certificates in the chain.
 * trusted_roots: array of trusted root certificates.
 * num_roots: number of trusted roots.
 * hostname: expected hostname for SAN/CN matching (NULL skips the check).
 * Returns 0 on success, or one of:
 *   ERROR_CODE_TLS_CERTIFICATE_EXPIRED
 *   ERROR_CODE_TLS_CERTIFICATE_INVALID
 *   ERROR_CODE_TLS_HOSTNAME_MISMATCH
 * The current wall-clock time is used for validity checks.
 */
int fun_crypto_x509_validate_chain(const fun_crypto_x509_cert *certs,
								   size_t num_certs,
								   const fun_crypto_x509_cert *trusted_roots,
								   size_t num_roots, const char *hostname);

/*
 * Same as fun_crypto_x509_validate_chain, but checks certificate validity
 * against an explicit `now_unix` timestamp. Useful for deterministic tests.
 */
int fun_crypto_x509_validate_chain_at(const fun_crypto_x509_cert *certs,
									  size_t num_certs,
									  const fun_crypto_x509_cert *trusted_roots,
									  size_t num_roots, const char *hostname,
									  int64_t now_unix);

/* ------------------------------------------------------------------
 * PEM parsing
 * ------------------------------------------------------------------ */

/*
 * Parse a PEM file containing X.509 certificates.
 * pem_data: raw PEM file content (need not be NUL-terminated).
 * pem_len: length of PEM data.
 * der_outputs: caller-provided array of DER output buffers.
 * der_sizes: capacity of each der_outputs[i] on input; on output the
 *   number of DER bytes written to that buffer.
 * max_certs: number of entries in der_outputs / der_sizes.
 * Returns the number of certificates extracted, or -1 on parse error
 * (malformed PEM or an output buffer too small).
 */
int fun_crypto_pem_parse(const char *pem_data, size_t pem_len,
						 uint8_t *der_outputs[], size_t der_sizes[],
						 size_t *num_certs, size_t max_certs);

#endif /* LIBRARY_CRYPTO_H */