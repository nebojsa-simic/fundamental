## ADDED Requirements

### Requirement: TCP connection can be established asynchronously
The system SHALL provide `fun_network_tcp_connect(address, &conn)` that initiates a non-blocking TCP connection and returns an `AsyncResult`. On `ASYNC_COMPLETED`, `*conn` SHALL be a valid `TcpNetworkConnection` handle. On `ASYNC_ERROR`, `*conn` SHALL be NULL.

#### Scenario: Successful connect sets handle
- **WHEN** `fun_network_tcp_connect` is called with a reachable address and the result is awaited
- **THEN** the `AsyncResult` status SHALL be `ASYNC_COMPLETED` and `*conn` SHALL be a non-NULL handle

#### Scenario: Unreachable host yields error
- **WHEN** `fun_network_tcp_connect` is called with an address whose host is unreachable and the result is awaited
- **THEN** the `AsyncResult` status SHALL be `ASYNC_ERROR` and `*conn` SHALL be NULL

#### Scenario: Connect returns immediately without blocking
- **WHEN** `fun_network_tcp_connect` is called
- **THEN** the function SHALL return an `AsyncResult` immediately; blocking occurs only inside `fun_async_await`

### Requirement: Data can be sent over a TCP connection asynchronously
The system SHALL provide `fun_network_tcp_send(conn, request)` that enqueues `request.data[0..request.length)` for transmission and returns an `AsyncResult`. On `ASYNC_COMPLETED` all bytes have been accepted by the kernel. The caller retains ownership of `request.data` for the duration of the async op.

#### Scenario: Send completes successfully
- **WHEN** `fun_network_tcp_send` is called on a connected handle and the result is awaited
- **THEN** the `AsyncResult` status SHALL be `ASYNC_COMPLETED` and all `request.length` bytes SHALL have been written to the socket

#### Scenario: Send on NULL connection returns error
- **WHEN** `fun_network_tcp_send` is called with a NULL `conn`
- **THEN** the returned `AsyncResult` SHALL have status `ASYNC_ERROR`

### Requirement: Exact byte count can be received asynchronously
The system SHALL provide `fun_network_tcp_receive_exact(conn, &response, bytes)` that reads exactly `bytes` octets into `response->data`. Surplus bytes returned by the OS beyond `bytes` SHALL be stored in the connection's internal overflow buffer and consumed by the next receive call. On `ASYNC_COMPLETED`, `response->length` SHALL equal `bytes`.

#### Scenario: Receive exact fills buffer completely
- **WHEN** `fun_network_tcp_receive_exact` is called with `bytes = N` and the remote sends at least N bytes
- **THEN** `response->length` SHALL equal N on `ASYNC_COMPLETED`

#### Scenario: Overflow bytes are consumed by subsequent receive
- **WHEN** the OS returns more bytes than requested and a second `receive_exact` is called
- **THEN** the surplus bytes from the first call SHALL be consumed before reading from the socket again

#### Scenario: Insufficient capacity returns error
- **WHEN** `fun_network_tcp_receive_exact` is called and `response->data` capacity is less than `bytes`
- **THEN** the `AsyncResult` SHALL have status `ASYNC_ERROR`

### Requirement: TCP connection can be closed
The system SHALL provide `fun_network_tcp_close(conn)` that closes the socket, frees the overflow buffer, and returns the connection slot to the internal reactor pool. The caller MUST NOT use `conn` after this call.

#### Scenario: Close releases resources
- **WHEN** `fun_network_tcp_close` is called on a valid handle
- **THEN** the socket SHALL be closed and the connection slot SHALL be returned to the pool with no resource leak

#### Scenario: Close on NULL is a no-op
- **WHEN** `fun_network_tcp_close` is called with a NULL handle
- **THEN** the function SHALL return without error and without any effect

### Requirement: Overflow buffer initial size is configurable via fun.ini
The system SHALL read `[network] rx_buf_size` from the configuration loaded by `fun_config_load()` at internal reactor init time. If the key is absent, a compile-time default (4096 bytes) SHALL be used.

#### Scenario: Default overflow buffer used when no config present
- **WHEN** no `fun.ini` is present or `[network]` does not contain `rx_buf_size`
- **THEN** each new connection's overflow buffer SHALL be initialised to 4096 bytes

#### Scenario: Config overrides overflow buffer initial size
- **WHEN** `fun.ini` contains `[network]\nrx_buf_size = 65536`
- **THEN** each new connection's overflow buffer SHALL be initialised to 65536 bytes

### Requirement: UDP datagram can be sent fire-and-forget
The system SHALL provide `fun_network_udp_send(address, datagram)` that creates an ephemeral UDP socket, sends `datagram` to `address`, and closes the socket. The caller retains ownership of `datagram.data` for the duration of the async op. No receive path is provided.

#### Scenario: UDP send completes successfully
- **WHEN** `fun_network_udp_send` is called with a valid address and non-empty datagram and the result is awaited
- **THEN** the `AsyncResult` status SHALL be `ASYNC_COMPLETED` and the datagram SHALL have been submitted to the OS

#### Scenario: UDP send to unreachable address returns error
- **WHEN** `fun_network_udp_send` is called with an address that cannot be reached at the socket layer
- **THEN** the `AsyncResult` status SHALL be `ASYNC_ERROR`
