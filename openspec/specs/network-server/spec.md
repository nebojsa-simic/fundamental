# network-server Specification

## Purpose
TBD - created by archiving change network-server-module. Update Purpose after archive.
## Requirements
### Requirement: TCP server config can be created with address and state
The system SHALL provide `fun_network_tcp_server_config(address, server_state, &out_config)` that creates a `NetworkServerConfig` for TCP server use. On success, `*out_config` is set to a valid config handle. On error, `*out_config` is unchanged. The `server_state` pointer is stored and passed to every listener callback invocation.

#### Scenario: Valid address creates config successfully
- **WHEN** `fun_network_tcp_server_config` is called with a valid `NetworkAddress` and non-NULL state pointer
- **THEN** the result SHALL be OK and `*out_config` SHALL be set to a valid `NetworkServerConfig` handle

#### Scenario: Invalid address returns error
- **WHEN** `fun_network_tcp_server_config` is called with an invalid address (e.g., port 0 for server use where prohibited by OS)
- **THEN** the function SHALL return an error result and `*out_config` SHALL be unchanged

#### Scenario: NULL output pointer returns error
- **WHEN** `fun_network_tcp_server_config` is called with a NULL `out_config`
- **THEN** the function SHALL return an error result

### Requirement: UDP server config can be created with address, state, and buffer
The system SHALL provide `fun_network_udp_server_config(address, server_state, buffer, buffer_size, &out_config)` that creates a `NetworkServerConfig` for UDP server use. The `buffer` is caller-allocated storage for incoming datagrams. `buffer_size` is the maximum datagram size; larger datagrams SHALL be truncated. On success, `*out_config` is set. On error, `*out_config` is unchanged.

#### Scenario: Valid address and buffer creates config successfully
- **WHEN** `fun_network_udp_server_config` is called with a valid address, non-NULL buffer, and buffer_size > 0
- **THEN** the result SHALL be OK and `*out_config` SHALL be set to a valid `NetworkServerConfig` handle

#### Scenario: NULL buffer returns error
- **WHEN** `fun_network_udp_server_config` is called with a NULL buffer
- **THEN** the function SHALL return an error result

#### Scenario: Zero buffer size returns error
- **WHEN** `fun_network_udp_server_config` is called with buffer_size = 0
- **THEN** the function SHALL return an error result

### Requirement: TCP listener callback receives client connection and state
The system SHALL invoke the `NetworkTcpListener` callback for each accepted TCP client connection. The callback receives a valid `TcpNetworkConnection` handle and the `server_state` pointer from config time. The callback SHALL be invoked on the internal accept thread.

#### Scenario: Callback invoked on accepted connection
- **WHEN** a TCP client connects to the listening address and the server is running
- **THEN** the `NetworkTcpListener` callback SHALL be invoked with a valid `TcpNetworkConnection` and the original `server_state`

#### Scenario: Callback receives correct state pointer
- **WHEN** the `NetworkTcpListener` callback is invoked
- **THEN** the `server_state` parameter SHALL equal the pointer passed to `fun_network_tcp_server_config`

### Requirement: UDP listener callback receives source, datagram, and state
The system SHALL invoke the `NetworkUdpListener` callback for each received UDP datagram. The callback receives the source `NetworkAddress`, a `NetworkBuffer` whose `data` points to the caller's config-time buffer and whose `length` is the actual bytes received (<= buffer_size), and the `server_state` pointer. The buffer SHALL be reused after the callback returns.

#### Scenario: Callback invoked on received datagram
- **WHEN** a UDP datagram is sent to the listening address and the server is running
- **THEN** the `NetworkUdpListener` callback SHALL be invoked with the source address, datagram buffer, and original `server_state`

#### Scenario: Datagram length does not exceed buffer size
- **WHEN** a datagram larger than buffer_size is received
- **THEN** the `NetworkBuffer.length` SHALL equal buffer_size and the data SHALL contain the first buffer_size bytes of the datagram

#### Scenario: Buffer is reused after callback returns
- **WHEN** the `NetworkUdpListener` callback returns and a second datagram arrives
- **THEN** the `NetworkBuffer.data` pointer SHALL point to the same caller-provided buffer with new contents

### Requirement: TCP listen returns AsyncResult with server lifetime semantics
The system SHALL provide `fun_network_tcp_listen(config, listener)` that starts the TCP accept loop on an internal thread and returns an `AsyncResult`. The result's status SHALL be:
- `ASYNC_ERROR` if the server failed to start (bind failed, port in use, etc.) — detectable immediately without awaiting
- `ASYNC_PENDING` while the server is running and accepting connections
- `ASYNC_COMPLETED` after `fun_network_server_stop()` is called and the accept thread has exited

#### Scenario: Bind failure returns ASYNC_ERROR immediately
- **WHEN** `fun_network_tcp_listen` is called with an address whose port is already in use
- **THEN** the returned `AsyncResult.status` SHALL be `ASYNC_ERROR` without awaiting

#### Scenario: Successful start returns ASYNC_PENDING
- **WHEN** `fun_network_tcp_listen` is called with a valid, available address
- **THEN** the returned `AsyncResult.status` SHALL be `ASYNC_PENDING` immediately

#### Scenario: Server transitions to COMPLETED after stop
- **WHEN** `fun_network_server_stop` is called and `fun_async_await` is invoked on the listen result
- **THEN** the result's status SHALL transition to `ASYNC_COMPLETED` after the accept thread exits

### Requirement: UDP listen returns AsyncResult with server lifetime semantics
The system SHALL provide `fun_network_udp_listen(config, listener)` that starts the UDP receive loop on an internal thread and returns an `AsyncResult`. The result's status SHALL follow the same semantics as TCP listen: `ASYNC_ERROR` on bind failure, `ASYNC_PENDING` while running, `ASYNC_COMPLETED` after stop.

#### Scenario: Bind failure returns ASYNC_ERROR immediately
- **WHEN** `fun_network_udp_listen` is called with an address whose port is already in use
- **THEN** the returned `AsyncResult.status` SHALL be `ASYNC_ERROR` without awaiting

#### Scenario: Successful start returns ASYNC_PENDING
- **WHEN** `fun_network_udp_listen` is called with a valid, available address
- **THEN** the returned `AsyncResult.status` SHALL be `ASYNC_PENDING` immediately

### Requirement: Server can be stopped gracefully
The system SHALL provide `fun_network_server_stop(config)` that signals the server to stop accepting new connections and closes the listen socket. The internal accept/receive thread SHALL exit cleanly after this call. Existing TCP client connections SHALL NOT be affected by this call.

#### Scenario: Stop closes listen socket
- **WHEN** `fun_network_server_stop` is called on a running server
- **THEN** the listen socket SHALL be closed and no new connections SHALL be accepted

#### Scenario: Stop on stopped server is safe
- **WHEN** `fun_network_server_stop` is called on a server that has already been stopped
- **THEN** the function SHALL return without error

### Requirement: Server config can be freed after shutdown
The system SHALL provide `fun_network_server_config_free(config)` that frees the config and all internal resources. The caller MUST call `fun_network_server_stop` and await the listen result before calling this function.

#### Scenario: Free after stop releases resources
- **WHEN** `fun_network_server_config_free` is called after `fun_network_server_stop` and the listen result has completed
- **THEN** all internal resources SHALL be freed with no memory leak

#### Scenario: Free on NULL is a no-op
- **WHEN** `fun_network_server_config_free` is called with a NULL config
- **THEN** the function SHALL return without error

### Requirement: TCP client connection handle is usable with existing network APIs
The `TcpNetworkConnection` handle passed to the `NetworkTcpListener` callback SHALL be compatible with existing TCP client functions: `fun_network_tcp_send`, `fun_network_tcp_receive_exact`, and `fun_network_tcp_close`.

#### Scenario: Client can send and receive
- **WHEN** the `NetworkTcpListener` callback uses `fun_network_tcp_receive_exact` and `fun_network_tcp_send` on the client handle
- **THEN** data SHALL be transmitted correctly between client and server

#### Scenario: Client can be closed
- **WHEN** `fun_network_tcp_close` is called on the client handle from the callback
- **THEN** the connection SHALL be closed and resources returned to the pool

### Requirement: Bound port can be queried after successful listen
The system SHALL provide `fun_network_server_get_port(config, &out_port)` that returns the actual port the server is bound to, in host byte order. This function MUST be called after listen succeeds; behavior before listen is undefined. Port 0 (ephemeral) allocation is supported — the OS-assigned port is returned.

#### Scenario: Port is returned after TCP listen
- **WHEN** `fun_network_tcp_listen` succeeds and `fun_network_server_get_port` is called
- **THEN** `out_port` SHALL contain the actual bound port number (1-65535)

#### Scenario: Port is returned after UDP listen
- **WHEN** `fun_network_udp_listen` succeeds and `fun_network_server_get_port` is called
- **THEN** `out_port` SHALL contain the actual bound port number (1-65535)

#### Scenario: Port 0 resolves to OS-assigned ephemeral port
- **WHEN** the server is configured with address port 0 and listen succeeds
- **THEN** `fun_network_server_get_port` SHALL return the OS-assigned port, not 0

### Requirement: SO_REUSEADDR is set on server sockets
The system SHALL set `SO_REUSEADDR` on all server sockets before calling `bind()`. This allows immediate restart without waiting for TIME_WAIT to expire.

#### Scenario: Server restarts on same port without delay
- **WHEN** a server is stopped and a new server is started on the same port
- **THEN** the second bind SHALL succeed without waiting for TIME_WAIT expiration

### Requirement: EINTR is handled in accept/receive loops
On Linux, the accept and receive loops SHALL retry on `EINTR` rather than treating it as a fatal error. On Windows, this requirement is not applicable (no EINTR semantics).

#### Scenario: Signal during accept does not stop the TCP server
- **WHEN** a signal is delivered to the accept thread during `accept()`
- **THEN** the accept loop SHALL retry `accept()` and the server SHALL continue running

#### Scenario: Signal during recvfrom does not stop the UDP server
- **WHEN** a signal is delivered to the receive thread during `recvfrom()`
- **THEN** the receive loop SHALL retry `recvfrom()` and the server SHALL continue running

### Requirement: TCP_NODELAY is set on accepted connections
The system SHALL set `TCP_NODELAY` on every accepted TCP client socket before invoking the `NetworkTcpListener` callback. This disables Nagle's algorithm for low-latency message delivery.

#### Scenario: Small sends are delivered immediately on accepted connections
- **WHEN** a TCP client connection is accepted and the callback sends a small message
- **THEN** the message SHALL be sent immediately without Nagle-induced delay

### Requirement: Config type is validated at listen time
The system SHALL store a protocol type tag in `NetworkServerConfig` at creation time. `fun_network_tcp_listen()` SHALL return `ASYNC_ERROR` with `ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE` if the config was created by `fun_network_udp_server_config()`. `fun_network_udp_listen()` SHALL return the same error if the config was created by `fun_network_tcp_server_config()`.

#### Scenario: TCP listen rejects UDP config
- **WHEN** `fun_network_tcp_listen` is called with a config created by `fun_network_udp_server_config`
- **THEN** the result status SHALL be `ASYNC_ERROR` with error code `ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE`

#### Scenario: UDP listen rejects TCP config
- **WHEN** `fun_network_udp_listen` is called with a config created by `fun_network_tcp_server_config`
- **THEN** the result status SHALL be `ASYNC_ERROR` with error code `ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE`

### Requirement: Stop blocks until current callback returns
When `fun_network_server_stop()` is called while a listener callback is executing on the internal thread, the stop function SHALL block until the callback returns. Calling `fun_network_server_stop()` from within a callback SHALL be safe: it sets the stop flag, and the thread exits after the callback returns without deadlock.

#### Scenario: Stop blocks during active callback
- **WHEN** `fun_network_server_stop` is called while a `NetworkTcpListener` callback is executing
- **THEN** `fun_network_server_stop` SHALL block until the callback returns

#### Scenario: Stop called from within callback is safe
- **WHEN** `fun_network_server_stop` is called from within a `NetworkTcpListener` callback
- **THEN** the call SHALL return without deadlock, the stop flag SHALL be set, and the thread SHALL exit after the callback returns

