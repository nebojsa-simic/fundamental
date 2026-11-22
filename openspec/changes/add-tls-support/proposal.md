## Why

The library has no encrypted transport — all network communication is plaintext. For the `fun init` CLI to auto-vendor from GitHub Releases, it needs TLS 1.2. Shelling out to `curl` or platform tools breaks the `-nostdlib` self-contained ethos.

## What Changes

- Add `crypto` module: SHA-256, HMAC-SHA256, AES-128-GCM, ECDH P-256, ECDSA P-256, X.509 certificate parsing and chain validation, PEM parsing
- Add `tls` module: TLS 1.2 record layer, client-mode handshake, PRF key derivation, SNI
- Modify `network` module: add `fun_network_tls_connect()` and `fun_network_tls_upgrade()` for async TLS-over-TCP
- Ship `certs/ca-bundle.pem` with baked-in root CAs; generate `tls_root_certs.c` at build time so no file is required at runtime
- Config cascade: TLS module reads `tls.ca_bundle` from global config to allow override of the default embedded roots

All crypto is implemented from scratch — zero external dependencies. No libcrypto, no mbedTLS, no Schannel.

## Capabilities

### New Capabilities
- `crypto-sha256`: SHA-256 hash function
- `crypto-hmac`: HMAC-SHA256
- `crypto-aes-gcm`: AES-128-GCM authenticated encryption and decryption
- `crypto-ecdh-p256`: ECDH key exchange on NIST P-256 (secp256r1)
- `crypto-ecdsa-p256`: ECDSA signature verification on P-256
- `crypto-x509`: X.509 certificate parsing (DER), full chain validation with SAN/CN hostname matching
- `crypto-pem`: PEM parsing (Base64 decode) for CA bundle files
- `tls-record`: TLS 1.2 record layer — fragment, encrypt, decrypt
- `tls-handshake`: TLS 1.2 client handshake — ClientHello through Finished
- `tls-prf`: TLS PRF key-derivation from master secret
- `tls-sni`: Server Name Indication extension

### Modified Capabilities
- `network`: new `fun_network_tls_connect(addr, hostname, out_conn)` and `fun_network_tls_upgrade(conn, hostname)` async functions

## Impact

- Single cipher suite: `TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256`
- No external dependencies — all crypto implemented from scratch, freestanding
- TLS module depends on `network`, `crypto`, `config` modules
- Caller-allocated memory pattern throughout; no allocations hidden from the caller
- Async operations follow existing `AsyncResult` pattern with poll functions
- Default root CAs baked into binary; `tls.ca_bundle` config key for runtime override
- New `certs/` directory at library root for CA bundle PEM file
- New build step: `generate-root-certs` script converts PEM to C array
- Non-goals: TLS 1.3, server-side TLS, client certificates, session resumption, compression, renegotiation, other cipher suites