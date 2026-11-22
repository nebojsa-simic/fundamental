# crypto Specification

## Purpose
Freestanding cryptographic primitives for the Fundamental Library: SHA-256 hashing, HMAC-SHA256 message authentication, AES-128 block encryption, AES-128-GCM authenticated encryption, 256-bit big-number modular arithmetic, EC P-256 point operations (ECDH key exchange and ECDSA signature verification), X.509 certificate parsing and chain validation, and PEM encoding/decoding. All code lives in `src/crypto/` with zero platform dependencies — pure computation, no I/O. The public API is declared in `include/fundamental/crypto/crypto.h`.

## ADDED Requirements

### Requirement: SHA-256 hashing
The system SHALL implement `fun_crypto_sha256(const uint8_t *input, size_t len, uint8_t output[32])` per FIPS 180-4, producing a 32-byte hash digest. The caller provides the output buffer. The implementation SHALL handle empty input, byte-aligned input, and bit-aligned input with correct padding.

#### Scenario: Empty input
- **GIVEN** an empty byte string (length 0)
- **WHEN** `fun_crypto_sha256` is called
- **THEN** the output SHALL be `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` (FIPS 180-4, one-block message)

#### Scenario: Three-byte input "abc"
- **GIVEN** the byte string `abc` (ASCII, 3 bytes)
- **WHEN** `fun_crypto_sha256` is called
- **THEN** the output SHALL be `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` (FIPS 180-4, one-block message)

#### Scenario: 448-bit input
- **GIVEN** the byte string `abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq` (ASCII, 448 bits / 56 bytes)
- **WHEN** `fun_crypto_sha256` is called
- **THEN** the output SHALL be `248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1` (FIPS 180-4, two-block message)

### Requirement: HMAC-SHA256 message authentication
The system SHALL implement `fun_crypto_hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *message, size_t message_len, uint8_t output[32])` per RFC 2104, producing a 32-byte authentication tag. The implementation SHALL handle keys shorter than, equal to, and longer than the SHA-256 block size (64 bytes).

#### Scenario: RFC 4231 Test Case 1
- **GIVEN** key = `0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b` (20 bytes) and message = `4869205468657265` ("Hi There")
- **WHEN** `fun_crypto_hmac_sha256` is called
- **THEN** the output SHALL be `b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7` (RFC 4231, key shorter than block size)

#### Scenario: RFC 4231 Test Case 2
- **GIVEN** key = `4a656665` ("Jefe") and message = `7768617420646f2079612077616e7420666f72206e6f7468696e673f` ("what do ya want for nothing?")
- **WHEN** `fun_crypto_hmac_sha256` is called
- **THEN** the output SHALL be `5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843` (RFC 4231)

#### Scenario: RFC 4231 Test Case 3
- **GIVEN** key = `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa` (131 bytes, longer than block size) and message = `54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a65204b6579202d2048617368204b6579204669727374` ("Test Using Larger Than Block-Size Key - Hash Key First")
- **WHEN** `fun_crypto_hmac_sha256` is called
- **THEN** the output SHALL be `60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54` (RFC 4231)

### Requirement: AES-128 block encryption
The system SHALL implement `fun_crypto_aes128_encrypt(const uint8_t key[16], const uint8_t plaintext[16], uint8_t ciphertext[16])` per FIPS-197 (Nk=4, Nr=10). The implementation SHALL use byte-oriented state matrix with SubBytes, ShiftRows, MixColumns, and AddRoundKey transformations.

#### Scenario: FIPS-197 Appendix B example
- **GIVEN** key = `2b7e151628aed2a6abf7158809cf4f3c` and plaintext = `3243f6a8885a308d313198a2e0370734`
- **WHEN** `fun_crypto_aes128_encrypt` is called
- **THEN** the ciphertext SHALL be `3925841d02dc09fbdc118597196a0b32` (FIPS-197 Appendix B, round-by-round example)

#### Scenario: Zero key encrypts zero plaintext
- **GIVEN** key = `00000000000000000000000000000000` and plaintext = `00000000000000000000000000000000`
- **WHEN** `fun_crypto_aes128_encrypt` is called
- **THEN** the ciphertext SHALL be `66e94bd4ef8a2c3b884cfa59ca342b2e` (known test vector for all-zero input)

### Requirement: AES-128-GCM authenticated encryption
The system SHALL implement `fun_crypto_aes128_gcm_encrypt(key, iv, iv_len, aad, aad_len, plaintext, pt_len, ciphertext, tag)` and `fun_crypto_aes128_gcm_decrypt(key, iv, iv_len, aad, aad_len, ciphertext, ct_len, tag, plaintext)` per NIST SP 800-38D. Encrypt SHALL produce a 16-byte authentication tag. Decrypt SHALL return 0 on tag match and non-zero on tag mismatch, with plaintext zeroed on failure. GHASH SHALL use bit-serial multiplication in GF(2^128) with polynomial x^128 + x^7 + x^2 + x + 1.

#### Scenario: Encrypt with known test vector
- **GIVEN** key = `00000000000000000000000000000000`, iv = `000000000000000000000000` (12 bytes), aad = empty, plaintext = empty
- **WHEN** `fun_crypto_aes128_gcm_encrypt` is called
- **THEN** the ciphertext SHALL be empty and the tag SHALL be `58e2fccefa7e3061367f1d57a4e7455a` (NIST CAVP, all-zero key/IV, empty input)

#### Scenario: Decrypt with valid tag
- **GIVEN** ciphertext and tag produced by `fun_crypto_aes128_gcm_encrypt` with the same key, IV, and AAD
- **WHEN** `fun_crypto_aes128_gcm_decrypt` is called
- **THEN** it returns 0 and the plaintext matches the original input

#### Scenario: Round-trip with non-empty plaintext
- **GIVEN** key = `feffe9928665731c6d6a8f9467308308`, iv = `cafebabefacedbaddecaf888` (12 bytes), aad = `feedfacedeadbeeffeedfacedeadbeefabaddad2` (20 bytes), plaintext = `d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a721c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b391aafd255` (64 bytes)
- **WHEN** `fun_crypto_aes128_gcm_encrypt` is called followed by `fun_crypto_aes128_gcm_decrypt`
- **THEN** decrypt returns 0 and the recovered plaintext matches the original

#### Scenario: Decrypt with tampered tag
- **GIVEN** a valid ciphertext and a tag where byte 0 has been flipped (tampered)
- **WHEN** `fun_crypto_aes128_gcm_decrypt` is called
- **THEN** it returns non-zero and the plaintext buffer is zeroed

#### Scenario: Decrypt with wrong key
- **GIVEN** a valid ciphertext and tag encrypted with key A
- **WHEN** `fun_crypto_aes128_gcm_decrypt` is called with key B (different from key A)
- **THEN** it returns non-zero

### Requirement: Variable-width big-number arithmetic
The system SHALL implement a `bignumber` module (`include/fundamental/bignumber/bignumber.h`, `src/bignumber/bignumber.c`) supporting 1..32 limbs of 32-bit little-endian values (i.e. 32 to 1024 bits), exposing `fun_bignumber_*` functions for add, subtract, compare, is_zero, is_one, is_odd, shift-right-one, wide multiply (schoolbook), modular add, modular subtract, modular multiply, and modular inverse. Modular multiplication SHALL use the fast NIST P-256 reduction when the modulus equals the secp256r1 prime (p = 2^256 - 2^224 + 2^192 + 2^96 - 1), and a generic long-division reduction for any other prime modulus (e.g. the secp256r1 group order n).

#### Scenario: Modular addition with wrap
- **GIVEN** a = p - 1 (the largest field element) and b = 2
- **WHEN** `fun_bignumber_modular_add` is called modulo p
- **THEN** the result SHALL be 1 (wrapping around the modulus)

#### Scenario: Modular subtraction with wrap
- **GIVEN** a = 1 and b = 2
- **WHEN** `fun_bignumber_modular_subtract` is called modulo p
- **THEN** the result SHALL be p - 1 (wrapping below zero)

#### Scenario: Modular multiplication with large values
- **GIVEN** a = p - 1 and b = p - 1
- **WHEN** `fun_bignumber_modular_multiply` is called modulo p
- **THEN** the result SHALL be 1 (since (p-1)^2 ≡ 1 mod p)

#### Scenario: Modular multiplication modulo a non-P-256 prime
- **GIVEN** a prime modulus other than the P-256 field prime (e.g. the group order n)
- **WHEN** `fun_bignumber_modular_multiply` is called
- **THEN** the generic reduction SHALL produce the correct product modulo that modulus

#### Scenario: Modular inverse
- **GIVEN** a = 3
- **WHEN** `fun_bignumber_modular_inverse` is called modulo p
- **THEN** the result multiplied by 3 modulo p SHALL equal 1 (inverse exists and is correct)

#### Scenario: Modular inverse of p-1
- **GIVEN** a = p - 1
- **WHEN** `fun_bignumber_modular_inverse` is called modulo p
- **THEN** the result SHALL be p - 1 (since (-1)^-1 ≡ -1 mod p)

### Requirement: EC P-256 point operations
The system SHALL implement point arithmetic on secp256r1 (NIST P-256) curve y² = x³ - 3x + b over F_p with b = 0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b. Operations SHALL include point doubling, point addition, and scalar multiplication (double-and-add) using affine coordinates. The generator point G SHALL be the standard NIST P-256 base point.

#### Scenario: Point doubling 2*G
- **GIVEN** the NIST P-256 generator point G
- **WHEN** the point doubling function is applied to G
- **THEN** the result SHALL be the known 2*G point with x-coordinate `7cf27b188d034f7e8a52380304b51ac3c08969e277f21b35a60b48fc47669978` and y-coordinate `07775510db8ed040293d9ac69f7430dbba7dade63ce982299e04b79d227873d1` (NIST P-256 test vector)

#### Scenario: Scalar multiplication 3*G
- **GIVEN** the NIST P-256 generator point G and scalar 3
- **WHEN** scalar multiplication is performed
- **THEN** the result SHALL be the known 3*G point

#### Scenario: Scalar multiplication by zero
- **GIVEN** any valid point P and scalar 0
- **WHEN** scalar multiplication is performed
- **THEN** the result SHALL be the point at infinity

### Requirement: ECDH shared secret computation
The system SHALL implement `int fun_crypto_ecdh_p256(const uint8_t private_key[32], const fun_crypto_ec_point *peer_public, uint8_t shared_secret[32])` computing the ECDH shared secret as the x-coordinate of scalar_multiply(private_key, peer_public). It SHALL return 0 on success and -1 on failure. Failure conditions SHALL include: private key zero or >= the group order n, peer point not on the curve, peer point at infinity, peer coordinate >= the field prime p, or a resulting shared point at infinity. On failure the shared secret SHALL be zeroed and MUST NOT be used to derive keys.

#### Scenario: ECDH test vector
- **GIVEN** private key = 2 and peer public key = the standard P-256 generator point G
- **WHEN** `fun_crypto_ecdh_p256` is called
- **THEN** it SHALL return 0 and the shared secret x-coordinate SHALL equal the x-coordinate of 2*G = `7cf27b188d034f7e8a52380304b51ac3c08969e277f21b35a60b48fc47669978`

#### Scenario: ECDH rejects an invalid peer point
- **GIVEN** a peer public key that is the point at infinity, off the curve, or has a coordinate >= p
- **WHEN** `fun_crypto_ecdh_p256` is called
- **THEN** it SHALL return -1 and write an all-zero shared secret

#### Scenario: ECDH symmetry
- **GIVEN** a key pair (priv_A, pub_A) and a key pair (priv_B, pub_B) generated on the P-256 curve
- **WHEN** ECDH is computed as `ecdh(priv_A, pub_B)` and `ecdh(priv_B, pub_A)`
- **THEN** both shared secrets SHALL be equal

### Requirement: ECDSA signature verification
The system SHALL implement `int fun_crypto_ecdsa_verify(const uint8_t hash[32], const fun_crypto_ec_point *public_key, const uint8_t sig_r[32], const uint8_t sig_s[32])` returning 0 for valid signatures and non-zero for invalid. Verification SHALL reject a public key not on the curve, require r and s in [1, n), compute u1 = hash * s^{-1} mod n, u2 = r * s^{-1} mod n using the group order n, compute u1*G + u2*Q, and compare the x-coordinate with r modulo n.

#### Scenario: Verify valid signature
- **GIVEN** a valid ECDSA P-256 signature (r, s) over a SHA-256 hash produced by the corresponding private key
- **WHEN** `fun_crypto_ecdsa_verify_p256` is called with the correct public key
- **THEN** it returns 0 (signature accepted)

#### Scenario: Reject tampered signature
- **GIVEN** a valid ECDSA P-256 signature where byte 0 of `r` has been flipped
- **WHEN** `fun_crypto_ecdsa_verify_p256` is called with the correct public key
- **THEN** it returns non-zero (signature rejected)

#### Scenario: Reject wrong public key
- **GIVEN** a valid ECDSA P-256 signature produced by key pair A
- **WHEN** `fun_crypto_ecdsa_verify_p256` is called with public key B (different from A)
- **THEN** it returns non-zero (signature rejected)

#### Scenario: Reject r out of range
- **GIVEN** an ECDSA signature where r = 0 or r ≥ n (the curve order)
- **WHEN** `fun_crypto_ecdsa_verify_p256` is called
- **THEN** it returns non-zero (signature rejected per SEC 1, Section 4.1.3)

### Requirement: X.509 certificate parsing
The system SHALL implement a recursive ASN.1 DER decoder capable of parsing X.509 v3 certificates into a minimal structure containing serial number, issuer DN, subject DN, notBefore, notAfter, subject public key info, extensions (SAN, Basic Constraints), signature algorithm, and signature value. The parser SHALL support INTEGER, BIT STRING, OCTET STRING, OID, UTCTime, GeneralizedTime, SEQUENCE, SET, PrintableString, UTF8String, IA5String, BMPString, NULL, and BOOLEAN tag types.

#### Scenario: Parse valid self-signed certificate
- **GIVEN** a DER-encoded X.509 v3 self-signed certificate
- **WHEN** the X.509 parser processes the certificate
- **THEN** the parsed structure contains the correct subject DN, public key, and validity period

#### Scenario: Extract Subject Alternative Name
- **GIVEN** a certificate with a SAN extension containing `dns:example.com`
- **WHEN** the X.509 parser extracts the SAN extension
- **THEN** the parsed SAN list contains `example.com`

#### Scenario: Parse certificate with Basic Constraints
- **GIVEN** a CA certificate with Basic Constraints CA:TRUE
- **WHEN** the X.509 parser extracts the extension
- **THEN** the parsed CA flag is true and the path length constraint is recorded

#### Scenario: Reject malformed DER
- **GIVEN** a truncated DER sequence (length exceeds available data)
- **WHEN** the X.509 parser processes it
- **THEN** it returns `ERROR_CODE_X509_PARSE_ERROR`

### Requirement: Certificate chain validation
The system SHALL implement `fun_crypto_x509_verify_chain(cert, ca_bundle)` that verifies each certificate's signature against the next certificate's public key, checks validity periods, and walks the chain to a trusted root in the provided CA bundle.

#### Scenario: Valid chain to trusted root
- **GIVEN** a leaf certificate signed by an intermediate CA, which is signed by a root CA present in the bundle
- **WHEN** `fun_crypto_x509_verify_chain` is called
- **THEN** it returns 0 and the chain is accepted

#### Scenario: Expired certificate
- **GIVEN** a certificate whose notAfter date is in the past
- **WHEN** `fun_crypto_x509_verify_chain` is called
- **THEN** it returns `ERROR_CODE_TLS_CERTIFICATE_EXPIRED`

#### Scenario: Missing root CA
- **GIVEN** a leaf certificate whose root CA is not in the provided bundle
- **WHEN** `fun_crypto_x509_verify_chain` is called
- **THEN** it returns `ERROR_CODE_TLS_CERTIFICATE_INVALID`

#### Scenario: Hostname mismatch
- **GIVEN** a certificate whose SAN entries are `dns:other.com` and no CN matches
- **WHEN** hostname validation is performed against `example.com`
- **THEN** it returns `ERROR_CODE_TLS_HOSTNAME_MISMATCH`

### Requirement: PEM parsing
The system SHALL implement `fun_crypto_pem_parse` that decodes Base64 data between `-----BEGIN CERTIFICATE-----` and `-----END CERTIFICATE-----` markers, supporting multiple certificates in a single file (CA bundle). The implementation SHALL handle standard PEM line wrapping (64 characters per line).

#### Scenario: Parse CA bundle with multiple certificates
- **GIVEN** a PEM file containing 3 certificates (root, intermediate, leaf) with proper BEGIN/END markers
- **WHEN** `fun_crypto_pem_parse` is called
- **THEN** it returns 3 DER-encoded certificate blobs

#### Scenario: Skip non-PEM content
- **GIVEN** a PEM file with plaintext lines before and after the certificate markers
- **WHEN** `fun_crypto_pem_parse` is called
- **THEN** the non-PEM text is ignored and only the Base64 between markers is decoded

#### Scenario: Reject malformed PEM
- **GIVEN** a PEM file with `-----BEGIN CERTIFICATE-----` but a missing or mismatched `-----END CERTIFICATE-----`
- **WHEN** `fun_crypto_pem_parse` is called
- **THEN** it returns `ERROR_CODE_PEM_PARSE_ERROR`

### Requirement: Embedded root CA store
The system SHALL provide a build-time tool `scripts/generate-root-certs.c` that reads `certs/ca-bundle.pem` and generates `src/crypto/tls_root_certs.c` as a static C array of DER-encoded root certificates. At runtime, the TLS module SHALL use embedded roots unless `tls.ca_bundle` is configured in the global config.

#### Scenario: Embedded roots used by default
- **GIVEN** no `tls.ca_bundle` config entry
- **WHEN** the TLS module initializes its root CA store
- **THEN** the embedded `g_tls_default_roots` array is used for chain validation

#### Scenario: Config-specified CA bundle overrides embedded roots
- **GIVEN** `tls.ca_bundle` = `custom-certs.pem` in the global config
- **WHEN** the TLS module initializes its root CA store
- **THEN** `custom-certs.pem` is parsed and used instead of the embedded roots

### Requirement: Crypto module is freestanding
All code in `src/crypto/` SHALL compile with `-nostdlib` and SHALL not use any C standard library functions (no malloc, no stdio, no string.h). All memory SHALL be caller-allocated. The only dependency SHALL be `include/fundamental/memory/memory.h` for `fun_memory_compare` and `fun_memory_copy`.

#### Scenario: Crypto sources compile with -nostdlib
- **GIVEN** a build script compiling all `src/crypto/*.c` files with `-nostdlib -ffreestanding` flags
- **WHEN** `gcc` compiles the translation units
- **THEN** no undefined references to stdlib functions are reported and each unit compiles cleanly

### Requirement: Public API header
The system SHALL provide `include/fundamental/crypto/crypto.h` declaring all crypto API functions with their complete signatures and necessary type definitions for key sizes, buffer sizes, and error codes.

#### Scenario: Header declares SHA-256 and HMAC
- **WHEN** the crypto header is included
- **THEN** `fun_crypto_sha256` and `fun_crypto_hmac_sha256` are declared with correct signatures

#### Scenario: Header declares AES and AES-GCM
- **WHEN** the crypto header is included
- **THEN** `fun_crypto_aes128_encrypt`, `fun_crypto_aes128_gcm_encrypt`, and `fun_crypto_aes128_gcm_decrypt` are declared with correct signatures

#### Scenario: Header declares EC and ECDSA
- **WHEN** the crypto header is included
- **THEN** `fun_crypto_ecdh_p256` and `fun_crypto_ecdsa_verify` are declared with correct signatures

#### Scenario: Header declares X.509 and PEM
- **WHEN** the crypto header is included
- **THEN** `fun_crypto_x509_verify_chain` and `fun_crypto_pem_parse` are declared with correct signatures