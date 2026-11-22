# Tasks: add-tls-support

## 1. Crypto module — core primitives

- [x] 1.1 SHA-256 implementation (`src/crypto/sha256.c`, `include/fundamental/crypto/crypto.h`)
- [x] 1.2 HMAC-SHA256 (`src/crypto/hmac.c`)
- [x] 1.3 AES block cipher encrypt (`src/crypto/aes.c`): 128-bit, byte-oriented state matrix, S-box lookup
- [x] 1.4 AES-GCM authenticated encryption + decryption (`src/crypto/aes_gcm.c`): CTR mode + GHASH (GF(2^128) multiplier)
- [x] 1.5 Variable-width big-number module (`include/fundamental/bignumber/`, `src/bignumber/`): add, subtract, compare, wide multiply, modular add/subtract/multiply (NIST P-256 fast reduction + generic reduction for any prime modulus), modular inverse (Fermat)
- [x] 1.6 ECDH/ECDSA on P-256 (`src/crypto/ec_p256.c`): point add/double (affine), scalar multiply (double-and-add), public-key validation, ECDH shared secret, ECDSA verify (mod group order n)
- [x] 1.7 X.509 DER parser (`src/crypto/x509.c`): ASN.1 parser, certificate structure extraction, chain validation, SAN/CN hostname matching
- [x] 1.8 PEM parser (`src/crypto/pem.c`): Base64 decode, certificate extraction

## 2. TLS module

- [ ] 2.1 TLS 1.2 record layer (`src/tls/tls_record.c`): fragment, encrypt (write), decrypt (read)
- [ ] 2.2 TLS PRF key derivation (`src/tls/tls_prf.c`): P_hash via HMAC-SHA256, master secret, key block expansion
- [ ] 2.3 TLS 1.2 client handshake (`src/tls/tls_handshake.c`): state machine (ClientHello → Finished), SNI extension, ECDHE key exchange
- [ ] 2.4 TLS module header (`include/fundamental/tls/tls.h`): public API types

## 3. Root CA infrastructure

- [ ] 3.1 Create `certs/ca-bundle.pem` with ~10 common root CAs
- [ ] 3.2 Build-time script `scripts/generate-root-certs` that converts PEM to `src/crypto/tls_root_certs.c`
- [ ] 3.3 Config integration: read `tls.ca_bundle` from global config; fall back to embedded roots

## 4. Network module integration

- [ ] 4.1 Add `fun_network_tls_connect()` to `include/fundamental/network/network.h` and `src/network/network.c`
- [ ] 4.2 Add `fun_network_tls_upgrade()` for upgrading existing TCP connections
- [ ] 4.3 Async poll function: chains TCP connect → TLS handshake → connected state

## 5. Error codes

- [x] 5.1 Register TLS/crypto error codes in `include/fundamental/error/error.h` (600–621)

## 6. Testing

- [x] 6.1 Crypto unit tests (`tests/crypto/`): SHA-256 (NIST vectors incl. 448-bit), HMAC (RFC 4231), AES (FIPS-197 + zero), AES-GCM (CAVP + 64-byte/AAD round-trip + bad-tag zeroing), bignumber (incl. modinv), ec_p256 (2*G), ECDH, ECDSA (valid + tampered), PEM (single/bundle/malformed), X.509 (parse leaf/root, chain valid/mismatch/expired/wrong root/tampered)
- [ ] 6.2 TLS integration tests (`tests/tls/`): handshake against local test server, GitHub TLS verification
- [x] 6.3 Build scripts for crypto + bignumber test directories (Windows + Linux)

## 7. Build & integration

- [ ] 7.1 Add crypto and tls source files to the test build scripts
- [ ] 7.2 Verify -nostdlib compilation on both platforms
- [ ] 7.3 Run full test suite (`run-tests-windows-amd64.bat`, `run-tests-linux-amd64.sh`)