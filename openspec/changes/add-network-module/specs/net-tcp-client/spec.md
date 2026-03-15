## ADDED Requirements

### Requirement: TCP connection can be initiated
The system SHALL provide a `fun_network_tcp_connect` function that begins a non-blocking TCP connection to a `NetworkAddress` and registers it with a `NetworkLoop`.

#### Scenario: Connection completes successfully
- **WHEN** `fun_network_tcp_connect` is called with a valid address and reachable host
- **THEN** the `on_connect` handler SHALL be invoked once the connection is established

#### Scenario: Connection fails with unreachable host
- **WHEN** `fun_network_tcp_connect` is called with an address whose host is unreachable
- **THEN** the `on_error` handler SHALL be invoked with an error code describing the failure

#### Scenario: Connection attempt is non-blocking
- **WHEN** `fun_network_tcp_connect` is called
- **THEN** the function SHALL return immediately without waiting for the TCP handshake to complete

### Requirement: Data can be sent over a TCP connection
The system SHALL provide a `fun_network_tcp_send` function that enqueues a single `NetworkBuffer` for transmission on a connected `NetworkConnection`.

#### Scenario: Send completes and notifies caller
- **WHEN** `fun_network_tcp_send` is called on a connected `NetworkConnection` and the data is successfully transmitted
- **THEN** the `on_write_complete` handler SHALL be invoked with the number of bytes sent

#### Scenario: Send on closed connection returns error
- **WHEN** `fun_network_tcp_send` is called on a `NetworkConnection` whose connection has been closed
- **THEN** `fun_network_tcp_send` SHALL return an error result and the `on_error` handler SHALL be invoked

### Requirement: Vectored data can be sent over a TCP connection
The system SHALL provide a `fun_network_tcp_send_vector` function that accepts a `NetworkBufferVector` and transmits all segments in order over a connected `NetworkConnection` using a single vectored syscall (`writev` / `WSASend` with multiple `WSABUF`s) where possible.

#### Scenario: Send-vector transmits all segments in order
- **WHEN** `fun_network_tcp_send_vector` is called with a `NetworkBufferVector` containing segments [A, B, C]
- **THEN** the bytes SHALL be transmitted in order A then B then C, and `on_write_complete` SHALL be invoked with the total byte count once all segments are sent

#### Scenario: Send-vector on closed connection returns error
- **WHEN** `fun_network_tcp_send_vector` is called on a `NetworkConnection` whose connection has been closed
- **THEN** `fun_network_tcp_send_vector` SHALL return an error result and the `on_error` handler SHALL be invoked

#### Scenario: Send-vector backpressure returns would-block
- **WHEN** `fun_network_tcp_send_vector` is called and the kernel send buffer is full
- **THEN** `fun_network_tcp_send_vector` SHALL return an error result with code `ERROR_CODE_NETWORK_WOULD_BLOCK`

### Requirement: Incoming data is delivered via callback with scatter receive
The system SHALL invoke the registered `on_read` handler whenever data arrives on a connected TCP connection. The caller SHALL provide a receive `NetworkBufferVector` at connect time; the arch layer SHALL use vectored read (`readv` / `WSARecv` with multiple `WSABUF`s) to scatter incoming bytes across the segments.

#### Scenario: Read callback delivers received bytes with scatter
- **WHEN** the remote end sends data and the loop processes the event
- **THEN** the `on_read` handler SHALL be called with the total number of bytes received, scattered across the caller's `NetworkBufferVector` segments in order

#### Scenario: Multiple read events deliver all data
- **WHEN** the remote end sends data in multiple segments
- **THEN** the `on_read` handler SHALL be called once per receive event, each time with the bytes available in that event

### Requirement: TCP connection can be closed
The system SHALL provide a `fun_network_tcp_close` function that initiates a graceful close of the connection.

#### Scenario: Close triggers on_close callback
- **WHEN** `fun_network_tcp_close` is called on a connected `NetworkConnection`
- **THEN** the TCP connection SHALL be gracefully closed and the `on_close` handler SHALL be invoked

#### Scenario: Remote close triggers on_close callback
- **WHEN** the remote end closes the connection
- **THEN** the `on_close` handler SHALL be invoked on the local `NetworkConnection`

### Requirement: Per-connection handlers are registered at connect time
The system SHALL accept a `NetworkHandlers` struct at `fun_network_tcp_connect` time specifying `on_connect`, `on_read`, `on_write_complete`, `on_close`, and `on_error` callbacks. The TCP `on_read` signature SHALL be `void (*on_read)(NetworkConnection *connection, NetworkBufferVector buffers, int bytes_received)`.

#### Scenario: All handlers are optional
- **WHEN** one or more handler pointers in `NetworkHandlers` are NULL
- **THEN** those events SHALL be silently ignored with no crash or undefined behaviour

### Requirement: Callbacks may call send, close, and stop re-entrantly
The system SHALL support calling `fun_network_tcp_send`, `fun_network_tcp_send_vector`, `fun_network_tcp_close`, and `fun_network_loop_stop` from within any callback (e.g., sending a response from inside `on_read`, or closing from inside `on_error`).

#### Scenario: Send from within on_read
- **WHEN** `fun_network_tcp_send` is called from within an `on_read` callback
- **THEN** the send SHALL be enqueued and processed in a subsequent dispatch cycle with no deadlock or undefined behaviour

#### Scenario: Close from within on_error
- **WHEN** `fun_network_tcp_close` is called from within an `on_error` callback
- **THEN** the connection SHALL be marked for close and the `on_close` handler SHALL be invoked after the current callback returns

#### Scenario: Stop from within a callback
- **WHEN** `fun_network_loop_stop` is called from within any connection callback
- **THEN** the loop SHALL exit after completing the current dispatch cycle

### Requirement: Write backpressure is signalled to caller
The system SHALL return `ERROR_CODE_NETWORK_WOULD_BLOCK` from `fun_network_tcp_send` and `fun_network_tcp_send_vector` when the kernel send buffer is full. The caller MUST wait for an `on_write_complete` callback before sending again.

#### Scenario: Send when buffer full returns would-block
- **WHEN** `fun_network_tcp_send` is called and the kernel send buffer cannot accept any bytes
- **THEN** `fun_network_tcp_send` SHALL return an error result with code `ERROR_CODE_NETWORK_WOULD_BLOCK`

#### Scenario: on_write_complete signals readiness to send again
- **WHEN** the kernel send buffer drains enough space after a would-block condition
- **THEN** the `on_write_complete` handler SHALL be invoked, signalling that the caller may call `fun_network_tcp_send` again

### Requirement: Partial writes are completed internally
The system SHALL handle TCP partial writes internally. When the kernel accepts fewer bytes than requested, the arch layer SHALL re-arm the write-ready event and continue sending the remainder. The `on_write_complete` callback SHALL only be invoked once all bytes from the original send call have been transmitted. This applies to both `fun_network_tcp_send` and `fun_network_tcp_send_vector`.

#### Scenario: Partial kernel write completes fully
- **WHEN** `fun_network_tcp_send` is called with N bytes and the kernel initially accepts fewer than N
- **THEN** the module SHALL continue sending the remainder on subsequent write-ready events and invoke `on_write_complete` only after all N bytes are sent

### Requirement: TCP connect supports a timeout
The system SHALL accept an optional `timeout_ms` parameter in `fun_network_tcp_connect`. If the connection is not established within the specified duration, the `on_error` handler SHALL be invoked with `ERROR_CODE_NETWORK_CONNECT_TIMEOUT`. A timeout value of 0 means no timeout (wait indefinitely).

#### Scenario: Connection times out
- **WHEN** `fun_network_tcp_connect` is called with `timeout_ms = 5000` and the remote host does not respond within 5 seconds
- **THEN** the `on_error` handler SHALL be invoked with `ERROR_CODE_NETWORK_CONNECT_TIMEOUT` and the connection attempt SHALL be cancelled

#### Scenario: No timeout when zero
- **WHEN** `fun_network_tcp_connect` is called with `timeout_ms = 0`
- **THEN** the connection attempt SHALL wait indefinitely until it succeeds or fails
