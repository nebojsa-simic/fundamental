## Architecture

Two new modules under a shared dependency chain:

```
include/fundamental/
  crypto/            # crypto primitives (pure math, no platform code)
    crypto.h
  tls/               # TLS 1.2 protocol (builds on crypto + network)
    tls.h

src/
  crypto/            # All freestanding, zero dependencies beyond the bignumber module
    sha256.c         SHA-256
    hmac.c           HMAC-SHA256 (built on SHA-256)
    aes.c            AES-128 block cipher (encrypt only)
    aes_gcm.c        AES-128-GCM mode (built on AES + GF(2^128))
    ec_p256.c        ECDH + ECDSA on secp256r1 (built on bignumber)
    x509.c           X.509 DER parsing and chain validation
    pem.c            PEM parsing (Base64 decode)
  bignumber/         # Variable-width (32..1024-bit) unsigned big-number arithmetic
    bignumber.c
  tls/
    tls_record.c     TLS 1.2 record layer
    tls_handshake.c  TLS 1.2 client handshake state machine
    tls_prf.c        TLS PRF key derivation (SHA-256 based)

certs/
    ca-bundle.pem    Mozilla root CA bundle (subset: ~10 common roots)
```

### Network module additions

```
include/fundamental/network/network.h
  + AsyncResult fun_network_tls_connect(NetworkAddress addr, const char *hostname, OutputTcpNetworkConnection out_conn);
  + AsyncResult fun_network_tls_upgrade(TcpNetworkConnection conn, const char *hostname);
```

---

## Crypto Module Design

### SHA-256

Implements SHA-256 per FIPS 180-4. Single-pass function: `fun_crypto_sha256(input, len, output[32])`. Caller provides 32-byte output buffer. Used internally by HMAC, TLS PRF, and hash-based message integrity.

### HMAC-SHA256

Implements HMAC per RFC 2104. Input: key, key_len, message, message_len. Output: 32-byte hash. Used by TLS PRF for key derivation.

### AES-128

Implements AES-128 encrypt per FIPS-197 (Nk=4). Block encrypt only — GCM mode handles the counter and authentication.

Design: byte-oriented state matrix `s[4][4]`, standard rounds (SubBytes, ShiftRows, MixColumns, AddRoundKey). Uses 256-byte S-box lookup table. No tables needed beyond S-box.

### AES-GCM

Implements AES-GCM per NIST SP 800-38D. Authenticated encryption (encrypt + tag). Needs:
- AES encrypt function for the counter blocks
- GF(2^128) multiplication for GHASH

The GHASH operation multiplies 128-bit blocks in GF(2^128) defined by the polynomial x^128 + x^7 + x^2 + x + 1. Implemented as bit-serial multiplication (simple but correct; adequate for handshake volumes).

API:
```c
void fun_crypto_aes128_gcm_encrypt(
    const uint8_t *key,          // 16 bytes (AES-128)
    const uint8_t *iv, size_t iv_len,       // typically 12
    const uint8_t *aad, size_t aad_len,
    const uint8_t *plaintext, size_t pt_len,
    uint8_t *ciphertext,                    // caller-allocated, pt_len bytes
    uint8_t *tag                            // caller-allocated, 16 bytes
);

int fun_crypto_aes128_gcm_decrypt(
    const uint8_t *key,          // 16 bytes
    const uint8_t *iv, size_t iv_len,
    const uint8_t *aad, size_t aad_len,
    const uint8_t *ciphertext, size_t ct_len,
    const uint8_t *tag,                      // 16-byte tag to verify
    uint8_t *plaintext                      // caller-allocated, ct_len bytes
);
// Returns 0 on success (tag verified), non-zero on failure.
```

### Big Number Arithmetic (bignumber)

256-bit unsigned integer arithmetic used by ECDH/ECDSA. Limb representation: 8 × 32-bit words, little-endian.

Operations needed:
- add, sub (with borrow chain)
- compare, is_zero, is_odd
- multiply (full 512-bit product via schoolbook multiplication)
- modular reduction (NIST P-256 fast reduction)
- modular inverse (extended Euclidean or Fermat)

The modular reduction uses the NIST P-256 formula:
`r = lo + hi * (2^224 - 2^192 - 2^96 + 1) mod p`
applied iteratively until the result fits in 256 bits.

### ECDH / ECDSA (P-256)

Implements point arithmetic on secp256r1 (NIST P-256).

Curve: y² = x³ - 3x + b over F_p, where p = 2^256 - 2^224 + 2^192 + 2^96 - 1.

Operations:
- Point addition and doubling (affine coordinates, lambda formula)
- Scalar multiplication (double-and-add)
- ECDSA signature verification: compute u1, u2 from hash, compute u1*G + u2*Q, compare x-coordinate with r

ECDH: scalar multiply peer's public point by own private key, use x-coordinate as shared secret.

For simplicity and correctness, affine coordinates are used (not Jacobian). Performance is secondary — handshakes are infrequent.

### X.509 Certificate Parsing

Parse DER-encoded X.509 certificates into a minimal structure:
- Serial number
- Issuer DN (for chain building)
- Subject DN
- Validity (notBefore, notAfter)
- Subject Public Key Info (algorithm, key bytes)
- Extensions: Subject Alternative Name, Basic Constraints
- Signature Algorithm + Signature Value

The parser is a recursive ASN.1 DER decoder supporting:
- INTEGER, BIT STRING, OCTET STRING, OID, UTCTime, GeneralizedTime
- SEQUENCE, SET
- PrintableString, UTF8String, IA5String, BMPString
- NULL, BOOLEAN

Chain validation:
1. Verify each certificate's signature against the next certificate's public key
2. Check validity periods
3. Verify hostname against SAN extension (or CN if no SAN)
4. Walk chain to a known root CA

### PEM Parser

Parse PEM-encoded files:
- Base64 decode between `-----BEGIN CERTIFICATE-----` and `-----END CERTIFICATE-----`
- Support multiple certificates in a single file (CA bundle)

### Root CA Store

A `certs/ca-bundle.pem` file with ~10 common root CAs (DigiCert Global Root, ISRG Root X1, GlobalSign Root CA, Amazon Root CA, etc.). A build script (`scripts/generate-root-certs.c` or similar) reads the PEM and generates `src/crypto/tls_root_certs.c` as a static C array, so roots are baked into the binary at compile time.

At runtime:
1. If `tls.ca_bundle` is set in config → parse the specified file
2. Otherwise → use the embedded `g_tls_default_roots` array

---

## TLS Module Design

### Supported Cipher Suite

`TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256` (0xC0, 0x2B)

### Record Layer

Each record: 5-byte header (content_type, version, length) + payload.

Record types:
- 20: ChangeCipherSpec
- 21: Alert
- 22: Handshake
- 23: Application Data

Plaintext records carry a sequence number for AEAD nonce construction. The GCM nonce is: 4 bytes salt (from ServerHello) + 8 bytes sequence number (big-endian, explicit).

After ChangeCipherSpec, all subsequent records use the negotiated cipher.

### Handshake State Machine

Client-mode handshake (TLS 1.2, no client auth):

```
STATE         SEND                            RECEIVE
--------      ----                            -------
CLIENT_HELLO  ClientHello           →
                                         ←    ServerHello
                                         ←    Certificate
                                         ←    ServerKeyExchange
                                         ←    ServerHelloDone
              ClientKeyExchange      →
              ChangeCipherSpec       →
              Finished               →
                                         ←    ChangeCipherSpec
                                         ←    Finished
CONNECTED     (application data)
```

Each state transition validates the received message and produces the appropriate response. The handshake is implemented as an async poll function that advances through states on each call.

### Key Derivation (TLS PRF)

TLS 1.2 PRF using SHA-256:

```
master_secret = PRF(pre_master_secret, "master secret", ClientHello.random + ServerHello.random)[0..47]
key_block = PRF(master_secret, "key expansion", ServerHello.random + ClientHello.random)

From key_block:
  client_write_key = key_block[0..15]          (AES-128 key)
  server_write_key = key_block[16..31]
  client_write_IV  = key_block[32..35]         (4-byte implicit nonce)
  server_write_IV  = key_block[36..39]
```

PRF: P_hash(secret, seed) using HMAC-SHA256, with the standard A(i) expansion:
```
A(0) = seed
A(i) = HMAC(secret, A(i-1))
P_hash = HMAC(secret, A(1) + seed) | HMAC(secret, A(2) + seed) | ...
```

### SNI Extension

ClientHello includes the `server_name` extension (type 0x0000) with the target hostname. This is required for virtual hosting (GitHub requires it).

### Async Poll Pattern

`fun_network_tls_connect` returns an `AsyncResult` whose `poll` function:
1. Starts TCP connect (reuses existing network connect)
2. On TCP completion, starts TLS handshake state machine
3. Each poll advances the handshake by sending/receiving one message
4. On handshake completion, marks the connection as TLS-enabled
5. Subsequent send/receive on the connection automatically encrypt/decrypt

The `poll` function uses the `TcpNetworkConnection.state` to track handshake progress (Epoch, cipher state, etc.).

### Config Dependency

The TLS module reads `tls.ca_bundle` from the global config at connection time:
```c
Config *config = fun_config_get_global();
StringResult ca_bundle = fun_config_get_string(config, "tls.ca_bundle");
if (fun_error_is_error(ca_bundle.error)) {
    // Use embedded default roots
}
```

---

## Error Codes

New error codes in `include/fundamental/error/error.h`:

```
ERROR_CODE_TLS_HANDSHAKE_FAILED     600
ERROR_CODE_TLS_BAD_RECORD_MAC       601
ERROR_CODE_TLS_CERTIFICATE_EXPIRED  602
ERROR_CODE_TLS_CERTIFICATE_INVALID  603
ERROR_CODE_TLS_HOSTNAME_MISMATCH    604
ERROR_CODE_TLS_UNSUPPORTED_CIPHER   605
ERROR_CODE_TLS_PROTOCOL_ERROR       606
ERROR_CODE_CRYPTO_KEY_TOO_SMALL     610
ERROR_CODE_CRYPTO_BAD_SIGNATURE     611
ERROR_CODE_CRYPTO_VERIFY_FAILED     612
ERROR_CODE_PEM_PARSE_ERROR          620
ERROR_CODE_X509_PARSE_ERROR         621
```

---

## File Layout

### New files

```
include/fundamental/crypto/crypto.h
src/crypto/sha256.c
src/crypto/hmac.c
src/crypto/aes.c
src/crypto/aes_gcm.c
src/bignumber/bignumber.c
src/crypto/ec_p256.c
src/crypto/x509.c
src/crypto/pem.c
src/crypto/tls_root_certs.c      (generated)
include/fundamental/tls/tls.h
src/tls/tls_record.c
src/tls/tls_handshake.c
src/tls/tls_prf.c
certs/ca-bundle.pem
scripts/generate-root-certs.c     (build-time tool)
```

### Modified files

```
include/fundamental/network/network.h   (add TLS functions)
src/network/network.c                   (add TLS connect/upgrade implementations)
include/fundamental/error/error.h       (add error codes)
tests/crypto/                           (new test directory)
tests/tls/                              (new test directory)
```

---

## Testing

### Unit tests (tests/crypto/)

- SHA-256: NIST test vectors (empty, "abc", 448-bit)
- HMAC: RFC 4231 test vectors
- AES: FIPS-197 test vectors (Appendix B)
- AES-GCM: NIST CAVP test vectors (cases 14, 15)
- bignumber: add/sub/mul/mod/inv with known values (incl. mod n)
- ec_p256: point doubling (2*G, 3*G), ECDH test vector, ECDSA verify
- x509: parse known certificates, chain validation
- pem: parse PEM file, extract certs

### Integration tests (tests/tls/)

- TLS handshake against local test server (openssl s_server)
- Verify GitHub TLS handshake succeeds
- Invalid hostname should fail
- Expired cert should fail
- Wrong CA should fail

### Build scripts

Each test directory gets platform-specific build scripts listing all needed source files.

---

## Platform Considerations

All crypto code is pure C with no OS dependencies — `src/crypto/` has no `arch/` directory. This is math, not I/O.

The TLS handshake uses `fun_network_tcp_send` and `fun_network_tcp_receive_exact` which already work cross-platform. No new platform code is needed.

The `scripts/generate-root-certs.c` is a build-time tool, not library code. It can use stdio/fopen.
