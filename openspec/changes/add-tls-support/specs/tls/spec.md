# tls Specification

## Purpose
TLS 1.2 client protocol implementation for the Fundamental Library: record layer encryption/decryption, PRF key derivation, client-mode handshake state machine, async network integration for TLS connect and upgrade, and certificate-based server authentication. The module builds on the crypto primitives (SHA-256, HMAC, AES-128-GCM, ECDH, ECDSA, X.509) and the network module's TCP transport. All code is freestanding and compiles with `-nostdlib`. The public API is declared in `include/fundamental/tls/tls.h`.

## ADDED Requirements

### Requirement: TLS 1.2 record layer
The system SHALL implement a TLS 1.2 record layer that encrypts and decrypts records using AES-128-GCM with the negotiated cipher suite `TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256`. Each record SHALL have a 5-byte header (content_type, version 0x0303, length). The AEAD nonce SHALL be constructed as 4 bytes of implicit write IV (salt) followed by 8 bytes of sequence number in big-endian. Sequence numbers SHALL be maintained separately for client and server directions.

#### Scenario: Encrypt application data record
- **GIVEN** the client write key, 4-byte client write IV, and sequence number 0
- **WHEN** an application data payload of 256 bytes is encrypted with content type 23
- **THEN** the record header contains content_type=23, version=0x0303, and the length reflects the ciphertext plus 16-byte tag
- **AND** the ciphertext and tag pass GCM decryption with the same key, IV, and sequence number

#### Scenario: Decrypt application data record
- **GIVEN** an encrypted record from the server (content type 23) with server write key, server write IV, and sequence number 0
- **WHEN** the record is decrypted
- **THEN** the plaintext matches the original payload and the record is accepted

#### Scenario: Detect tampered record
- **GIVEN** a valid encrypted record from the server
- **WHEN** byte 10 of the ciphertext is flipped and decryption is attempted
- **THEN** decryption returns `ERROR_CODE_TLS_BAD_RECORD_MAC`

#### Scenario: Detect record with wrong key
- **GIVEN** a record encrypted with server write key
- **WHEN** decryption is attempted using a different key
- **THEN** decryption returns `ERROR_CODE_TLS_BAD_RECORD_MAC`

#### Scenario: Handle unencrypted handshake records
- **GIVEN** the handshake is in progress (ChangeCipherSpec not yet received)
- **WHEN** a handshake record (content type 22) is received
- **THEN** the payload is processed as plaintext without AEAD decryption

### Requirement: TLS PRF key derivation
The system SHALL implement `fun_tls_prf_sha256(const uint8_t *secret, size_t secret_len, const uint8_t *label, size_t label_len, const uint8_t *seed, size_t seed_len, uint8_t *output, size_t output_len)` per TLS 1.2 (RFC 5246, Section 5). The PRF SHALL use P_hash with HMAC-SHA256 and the standard A(i) expansion: A(0) = seed, A(i) = HMAC(secret, A(i-1)), P_hash = HMAC(secret, A(1) || seed) || HMAC(secret, A(2) || seed) || ...

#### Scenario: Derive master secret
- **GIVEN** pre_master_secret = `03036db4f2e81f7e0d51011ae5963b25c0fd168ae4a727f1cf7e3bda28d9cf2c1df9153f0827b54a8cb7313b760cf7939affc0b288db5c68a85f74141423fa26` (48 bytes), label = "master secret", seed = client_random || server_random (64 bytes of known content)
- **WHEN** `fun_tls_prf_sha256` is called with output_len=48
- **THEN** the output SHALL be the master secret matching the TLS 1.2 test vector

#### Scenario: Derive key block
- **GIVEN** a 48-byte master secret, label = "key expansion", seed = server_random || client_random (64 bytes of known content)
- **WHEN** `fun_tls_prf_sha256` is called with output_len=40
- **THEN** the output contains: client_write_key (bytes 0-15), server_write_key (bytes 16-31), client_write_IV (bytes 32-35), server_write_IV (bytes 36-39)

#### Scenario: PRF with zero-length output
- **GIVEN** any valid secret, label, and seed
- **WHEN** `fun_tls_prf_sha256` is called with output_len=0
- **THEN** it returns immediately without writing to the output buffer

### Requirement: TLS 1.2 client handshake
The system SHALL implement a client-mode TLS 1.2 handshake state machine as an async poll function `fun_tls_handshake_poll`. The handshake SHALL negotiate `TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256`. States: send ClientHello, receive ServerHello, receive Certificate, receive ServerKeyExchange, receive ServerHelloDone, send ClientKeyExchange, send ChangeCipherSpec, send Finished, receive ChangeCipherSpec, receive Finished, connected.

#### Scenario: Complete handshake against compliant server
- **GIVEN** a TLS 1.2 server supporting `TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256` with a valid certificate chain
- **WHEN** the handshake state machine advances through all states
- **THEN** the connection reaches the CONNECTED state and session keys are established
- **AND** the Finished message verification succeeds in both directions

#### Scenario: ClientHello includes SNI extension
- **GIVEN** the target hostname is `example.com`
- **WHEN** the ClientHello message is constructed
- **THEN** it contains the server_name extension (type 0x0000) with hostname `example.com`

#### Scenario: ClientHello includes supported cipher suites
- **WHEN** the ClientHello message is constructed
- **THEN** it advertises `TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256` (0xC0, 0x2B) in the cipher suites list

#### Scenario: ClientHello includes supported elliptic curves
- **WHEN** the ClientHello message is constructed
- **THEN** it includes the supported_groups extension containing secp256r1 (0x0017)

#### Scenario: Server selects unsupported cipher suite
- **GIVEN** a server that responds with a cipher suite other than `TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256`
- **WHEN** the handshake processes ServerHello
- **THEN** it returns `ERROR_CODE_TLS_UNSUPPORTED_CIPHER`

#### Scenario: Certificate validation failure
- **GIVEN** a server presenting a certificate not trusted by the configured CA bundle
- **WHEN** the handshake validates the server certificate chain
- **THEN** it returns `ERROR_CODE_TLS_CERTIFICATE_INVALID`

#### Scenario: ECDHE key exchange
- **GIVEN** the server's ECDHE public key from ServerKeyExchange
- **WHEN** the ClientKeyExchange is constructed
- **THEN** it contains an ephemeral ECDHE public key on secp256r1
- **AND** the pre_master_secret is derived as the x-coordinate of the ECDH shared secret

#### Scenario: Handshake protocol error
- **GIVEN** a server that sends a handshake message with an unexpected type or out of sequence
- **WHEN** the handshake state machine processes the message
- **THEN** it returns `ERROR_CODE_TLS_PROTOCOL_ERROR`

#### Scenario: Server sends alert during handshake
- **GIVEN** a server that responds with a TLS Alert record (e.g., handshake_failure)
- **WHEN** the handshake state machine receives the alert
- **THEN** it returns `ERROR_CODE_TLS_HANDSHAKE_FAILED` with the alert description code

### Requirement: Network TLS connect
The system SHALL extend `include/fundamental/network/network.h` with `fun_network_tls_connect(NetworkAddress addr, const char *hostname, OutputTcpNetworkConnection out_conn)` returning an `AsyncResult` whose poll function establishes a TCP connection and then performs the TLS handshake transparently.

#### Scenario: Async TLS connect completes successfully
- **GIVEN** a reachable TLS server at address and hostname
- **WHEN** `fun_network_tls_connect` is called and the returned `AsyncResult` is polled to completion
- **THEN** `out_conn` contains a connected `TcpNetworkConnection` with TLS session keys established
- **AND** the connection status is `ASYNC_COMPLETED`

#### Scenario: TLS connect timeout
- **GIVEN** a server that accepts TCP but never responds to TLS ClientHello
- **WHEN** `fun_network_tls_connect` is polled and the configured timeout expires
- **THEN** the `AsyncResult` status is `ASYNC_ERROR` and the connection is cleaned up

#### Scenario: Subsequent send/receive on TLS connection uses encryption
- **GIVEN** a `TcpNetworkConnection` established via `fun_network_tls_connect` in CONNECTED state
- **WHEN** `fun_network_tcp_send` is called with application data
- **THEN** the data is encrypted as a TLS application data record before sending
- **WHEN** `fun_network_tcp_receive_exact` is called
- **THEN** the received bytes are decrypted from TLS records and the plaintext is returned

### Requirement: Network TLS upgrade
The system SHALL provide `fun_network_tls_upgrade(TcpNetworkConnection conn, const char *hostname)` that upgrades an existing plaintext TCP connection to TLS. The returned `AsyncResult` poll performs the TLS handshake over the existing connection.

#### Scenario: TLS upgrade succeeds
- **GIVEN** an established `TcpNetworkConnection` to a TLS server
- **WHEN** `fun_network_tls_upgrade` is called and polled to completion
- **THEN** the connection is upgraded to TLS and subsequent traffic is encrypted

#### Scenario: TLS upgrade on connection to non-TLS server
- **GIVEN** a TCP connection to a server that sends non-TLS data (e.g., HTTP plaintext)
- **WHEN** `fun_network_tls_upgrade` is polled and the handshake fails
- **THEN** the `AsyncResult` status is `ASYNC_ERROR` with `ERROR_CODE_TLS_HANDSHAKE_FAILED`

### Requirement: Config integration for TLS
The TLS module SHALL read `tls.ca_bundle` from the global config at connection time. If set, the specified PEM file SHALL be parsed for root CAs. If not set, the embedded default root CA array SHALL be used.

#### Scenario: Config provides custom CA bundle
- **GIVEN** `tls.ca_bundle` = `certs/custom-ca.pem` in the global config
- **WHEN** `fun_network_tls_connect` is called
- **THEN** the handshake validates the server certificate against roots from `custom-ca.pem`

#### Scenario: Config is absent, use embedded roots
- **GIVEN** no `tls.ca_bundle` entry in the global config
- **WHEN** `fun_network_tls_connect` is called
- **THEN** the handshake validates the server certificate against the embedded `g_tls_default_roots` array

#### Scenario: Config-specified bundle not found
- **GIVEN** `tls.ca_bundle` = `nonexistent.pem` and the file does not exist
- **WHEN** `fun_network_tls_connect` is called
- **THEN** it returns `ASYNC_ERROR` with an appropriate error code indicating the CA bundle could not be loaded

### Requirement: TLS module is freestanding
All TLS source files in `src/tls/` SHALL compile with `-nostdlib` and SHALL not use any C standard library functions. The module SHALL depend only on `include/fundamental/crypto/crypto.h`, `include/fundamental/network/network.h`, `include/fundamental/config/config.h`, and `include/fundamental/memory/memory.h`.

#### Scenario: TLS sources compile with -nostdlib
- **GIVEN** a build script compiling all `src/tls/*.c` files with `-nostdlib -ffreestanding` flags
- **WHEN** `gcc` compiles the translation units
- **THEN** no undefined references to stdlib functions are reported and each unit compiles cleanly

### Requirement: TLS public API header
The system SHALL provide `include/fundamental/tls/tls.h` declaring the TLS PRF function, handshake state machine entry points, record layer functions, and session state types.

#### Scenario: Header declares PRF
- **WHEN** the TLS header is included
- **THEN** `fun_tls_prf_sha256` is declared with correct signature

#### Scenario: Header declares record layer functions
- **WHEN** the TLS header is included
- **THEN** functions for TLS record encryption, decryption, and sequence number management are declared

#### Scenario: Header declares handshake types
- **WHEN** the TLS header is included
- **THEN** the `TlsHandshakeState` struct and `fun_tls_handshake_poll` function are declared

#### Scenario: Header includes error codes
- **WHEN** the TLS header is included
- **THEN** `ERROR_CODE_TLS_HANDSHAKE_FAILED`, `ERROR_CODE_TLS_BAD_RECORD_MAC`, `ERROR_CODE_TLS_CERTIFICATE_EXPIRED`, `ERROR_CODE_TLS_CERTIFICATE_INVALID`, `ERROR_CODE_TLS_HOSTNAME_MISMATCH`, `ERROR_CODE_TLS_UNSUPPORTED_CIPHER`, and `ERROR_CODE_TLS_PROTOCOL_ERROR` are defined

### Requirement: TLS error codes
The system SHALL define new error codes in `include/fundamental/error/error.h` for TLS and crypto failures:

| Code | Name | Value |
|------|------|-------|
| 600 | `ERROR_CODE_TLS_HANDSHAKE_FAILED` | 600 |
| 601 | `ERROR_CODE_TLS_BAD_RECORD_MAC` | 601 |
| 602 | `ERROR_CODE_TLS_CERTIFICATE_EXPIRED` | 602 |
| 603 | `ERROR_CODE_TLS_CERTIFICATE_INVALID` | 603 |
| 604 | `ERROR_CODE_TLS_HOSTNAME_MISMATCH` | 604 |
| 605 | `ERROR_CODE_TLS_UNSUPPORTED_CIPHER` | 605 |
| 606 | `ERROR_CODE_TLS_PROTOCOL_ERROR` | 606 |
| 610 | `ERROR_CODE_CRYPTO_KEY_TOO_SMALL` | 610 |
| 611 | `ERROR_CODE_CRYPTO_BAD_SIGNATURE` | 611 |
| 612 | `ERROR_CODE_CRYPTO_VERIFY_FAILED` | 612 |
| 620 | `ERROR_CODE_PEM_PARSE_ERROR` | 620 |
| 621 | `ERROR_CODE_X509_PARSE_ERROR` | 621 |

#### Scenario: Error codes are defined and unique
- **WHEN** `include/fundamental/error/error.h` is included
- **THEN** all 12 error codes are defined with the values shown in the table above
- **AND** no value conflicts with existing error codes

### Requirement: Build-time root certificate generation
The system SHALL provide `scripts/generate-root-certs.c`, a build-time tool that parses `certs/ca-bundle.pem` and generates `src/crypto/tls_root_certs.c` containing a static `const uint8_t *g_tls_default_roots[]` array of DER-encoded certificates plus a `const size_t g_tls_default_roots_len` count. The generated file SHALL compile as part of `src/crypto/` without linking to the tool binary.

#### Scenario: Generation succeeds with valid PEM
- **GIVEN** `certs/ca-bundle.pem` containing PEM-encoded root CA certificates
- **WHEN** `scripts/generate-root-certs.c` is compiled and run
- **THEN** it writes `src/crypto/tls_root_certs.c` with valid C syntax
- **AND** the generated array contains at least one DER blob per certificate in the bundle

#### Scenario: Generated file is checked in
- **WHEN** the source tree is cloned and built
- **THEN** `src/crypto/tls_root_certs.c` is present and compilable without running the generation tool