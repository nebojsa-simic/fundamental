## Why

Distributed systems require encrypted communication, but the library has no TLS/SSL support. Messages between brokers are plaintext, exposing credentials and data in transit.

## What Changes

- Add TLS/SSL support to network module
- Support certificate-based authentication
- Provide secure TCP connection variants
- Integrate with existing async network interface

## Capabilities

### New Capabilities
- `tls-handshake`: TLS handshake protocol implementation
- `tls-stream`: Encrypted stream wrapper for TCP connections
- `tls-certs`: Certificate loading and validation
- `tls-config`: TLS configuration (ciphers, protocols, verification)

### Modified Capabilities
- `network`: Network module will gain `fun_network_tls_connect()`

## Impact

- New dependency on TLS implementation (mbedTLS or similar)
- New functions: `fun_network_tls_connect()`, `fun_network_tls_upgrade()`
- Existing plaintext network functions remain unchanged
- Enables secure distributed system deployments
