# Network Module Specification

Non-blocking, event-driven TCP/UDP networking. Reactor model: epoll on Linux, IOCP on Windows. All memory is caller-allocated; no internal heap allocation on the data path.

---

## Event Loop

### Requirement: Event loop can be created and destroyed
The system SHALL provide a `fun_network_loop_init` function that initialises a caller-allocated `NetworkLoop` and a `fun_network_loop_destroy` function that releases all platform resources associated with it.

#### Scenario: Successful initialisation
- **WHEN** the caller provides a valid `NetworkLoop` buffer of at least `NETWORK_LOOP_SIZE` bytes
- **THEN** `fun_network_loop_init` SHALL return a result with no error and the loop SHALL be ready to accept connections

#### Scenario: Destroy releases resources
- **WHEN** `fun_network_loop_destroy` is called on an initialised loop
- **THEN** all platform handles (epoll fd / IOCP handle) SHALL be closed and no resources SHALL be leaked

### Requirement: Event loop can run in blocking mode
The system SHALL provide a `fun_network_loop_run` function that blocks the calling thread, dispatching I/O callbacks until `fun_network_loop_stop` is called.

#### Scenario: Run processes pending events
- **WHEN** `fun_network_loop_run` is called and I/O events are available
- **THEN** the corresponding connection callbacks SHALL be invoked before the function returns

#### Scenario: Run blocks until stopped
- **WHEN** `fun_network_loop_run` is called with no pending events
- **THEN** the function SHALL block until either an event arrives or `fun_network_loop_stop` is called from a callback

### Requirement: Event loop can tick with optional timeout
The system SHALL provide a `fun_network_loop_run_once` function that accepts a `timeout_ms` parameter. It SHALL wait up to `timeout_ms` milliseconds for I/O events, dispatch any ready callbacks, and return. A timeout of 0 means return immediately (pure poll). A timeout of -1 means block indefinitely until at least one event arrives.

#### Scenario: Run-once with zero timeout returns immediately
- **WHEN** `fun_network_loop_run_once` is called with `timeout_ms = 0` and no events are ready
- **THEN** the function SHALL return immediately without blocking

#### Scenario: Run-once with positive timeout waits then returns
- **WHEN** `fun_network_loop_run_once` is called with `timeout_ms = 100` and no events arrive within 100ms
- **THEN** the function SHALL return after approximately 100ms

#### Scenario: Run-once dispatches ready callbacks
- **WHEN** `fun_network_loop_run_once` is called and one or more connections have ready I/O
- **THEN** their callbacks SHALL be invoked before the function returns

#### Scenario: Run-once with negative-one timeout blocks until event
- **WHEN** `fun_network_loop_run_once` is called with `timeout_ms = -1` and no events are ready
- **THEN** the function SHALL block until at least one event arrives or `fun_network_loop_stop` is called

### Requirement: Event loop can be stopped
The system SHALL provide a `fun_network_loop_stop` function that signals the loop to exit its `fun_network_loop_run` call.

#### Scenario: Stop causes run to return
- **WHEN** `fun_network_loop_stop` is called from within an I/O callback while the loop is running
- **THEN** `fun_network_loop_run` SHALL return after completing the current dispatch cycle

---

## Address

### Requirement: IPv4 address can be parsed from string
The system SHALL provide a `fun_network_address_parse` function that parses an `"IPv4:port"` string (e.g., `"127.0.0.1:8080"`) into a `NetworkAddress`.

#### Scenario: Valid IPv4 address parses successfully
- **WHEN** `fun_network_address_parse` is called with a well-formed `"a.b.c.d:port"` string
- **THEN** the result SHALL contain a `NetworkAddress` with family `NETWORK_ADDRESS_IPV4`, the four address octets, and the port number

#### Scenario: Invalid IPv4 octet returns error
- **WHEN** `fun_network_address_parse` is called with an octet value greater than 255 (e.g., `"999.0.0.1:80"`)
- **THEN** `fun_network_address_parse` SHALL return an error result

#### Scenario: Missing port returns error
- **WHEN** `fun_network_address_parse` is called with a string that has no colon-port suffix
- **THEN** `fun_network_address_parse` SHALL return an error result

### Requirement: IPv6 address can be parsed from string
The system SHALL provide support for bracketed IPv6 notation (e.g., `"[::1]:8080"`) in `fun_network_address_parse`.

#### Scenario: Valid IPv6 address parses successfully
- **WHEN** `fun_network_address_parse` is called with a well-formed `"[addr]:port"` string
- **THEN** the result SHALL contain a `NetworkAddress` with family `NETWORK_ADDRESS_IPV6`, the 16 address bytes, and the port number

#### Scenario: Missing closing bracket returns error
- **WHEN** `fun_network_address_parse` is called with `"[::1:80"` (no closing bracket)
- **THEN** `fun_network_address_parse` SHALL return an error result

### Requirement: Port number is validated
The system SHALL validate that parsed port numbers are in the range 0–65535.

#### Scenario: Port 0 is accepted for OS-assigned port
- **WHEN** `fun_network_address_parse` is called with port 0
- **THEN** the result SHALL contain a `NetworkAddress` with the port set to 0

#### Scenario: Port 65535 is accepted
- **WHEN** `fun_network_address_parse` is called with port 65535
- **THEN** the result SHALL contain a `NetworkAddress` with the port set to 65535

#### Scenario: Port above 65535 returns error
- **WHEN** `fun_network_address_parse` is called with a port value greater than 65535
- **THEN** `fun_network_address_parse` SHALL return an error result

### Requirement: Address can be formatted to string
The system SHALL provide a `fun_network_address_to_string` function that writes a human-readable `"host:port"` representation of a `NetworkAddress` into a caller-supplied buffer.

#### Scenario: IPv4 address formats correctly
- **WHEN** `fun_network_address_to_string` is called with an IPv4 `NetworkAddress`
- **THEN** the output buffer SHALL contain `"a.b.c.d:port"` as a null-terminated string

#### Scenario: IPv6 address formats correctly
- **WHEN** `fun_network_address_to_string` is called with an IPv6 `NetworkAddress`
- **THEN** the output buffer SHALL contain `"[addr]:port"` as a null-terminated string

#### Scenario: Buffer too small returns error
- **WHEN** `fun_network_address_to_string` is called with a buffer smaller than the formatted string
- **THEN** the function SHALL return an error result and SHALL NOT write beyond the buffer boundary

### Requirement: Hostname strings are rejected
The system SHALL only accept numeric IP addresses. Strings containing non-numeric hostnames SHALL be rejected.

#### Scenario: Hostname string returns error
- **WHEN** `fun_network_address_parse` is called with a hostname string (e.g., `"example.com:80"`)
- **THEN** `fun_network_address_parse` SHALL return an error result

#### Scenario: localhost string returns error
- **WHEN** `fun_network_address_parse` is called with `"localhost:8080"`
- **THEN** `fun_network_address_parse` SHALL return an error result

---

## Buffer

### Requirement: NetworkBuffer wraps caller-owned memory
The system SHALL define a `NetworkBuffer` type that holds a pointer to caller-owned data and its length, with no internal allocation.

#### Scenario: NetworkBuffer can be constructed from a stack array
- **WHEN** a caller initialises a `NetworkBuffer` with a pointer to a stack-allocated array and its length
- **THEN** the `NetworkBuffer` SHALL refer to that memory and report the correct length

#### Scenario: NetworkBuffer with NULL pointer and zero length is valid as empty
- **WHEN** a `NetworkBuffer` is initialised with a NULL pointer and length 0
- **THEN** the buffer SHALL be considered empty and passing it to send/receive functions SHALL have no effect

### Requirement: NetworkBuffer can be sliced
The system SHALL provide a `fun_network_buffer_slice` function that returns a sub-`NetworkBuffer` starting at a given offset with a given length.

#### Scenario: Valid slice returns sub-buffer
- **WHEN** `fun_network_buffer_slice` is called with an offset and length that fit within the original buffer
- **THEN** the returned `NetworkBuffer` SHALL point to `original.data + offset` with the specified length

#### Scenario: Slice beyond buffer end returns error
- **WHEN** `fun_network_buffer_slice` is called with `offset + length > original.length`
- **THEN** `fun_network_buffer_slice` SHALL return an error result

### Requirement: NetworkBufferVector represents a scatter/gather vector of buffers
The system SHALL define a `NetworkBufferVector` type that holds a pointer to a caller-owned array of `NetworkBuffer` elements and a count, enabling vectored I/O (scatter/gather) across non-contiguous memory regions in a single syscall.

#### Scenario: NetworkBufferVector total length sums all segments
- **WHEN** `fun_network_buffer_vector_total_length` is called on a `NetworkBufferVector` with 3 buffers of lengths 100, 200, 300
- **THEN** the function SHALL return 600

#### Scenario: Empty vector total length is zero
- **WHEN** `fun_network_buffer_vector_total_length` is called on a vector with count 0
- **THEN** the function SHALL return 0

---

## TCP Client

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

### Requirement: TCP connect supports a timeout
The system SHALL accept an optional `timeout_ms` parameter in `fun_network_tcp_connect`. If the connection is not established within the specified duration, the `on_error` handler SHALL be invoked with `ERROR_CODE_NETWORK_CONNECT_TIMEOUT`. A timeout value of 0 means no timeout.

#### Scenario: Connection times out
- **WHEN** `fun_network_tcp_connect` is called with `timeout_ms = 5000` and the remote host does not respond within 5 seconds
- **THEN** the `on_error` handler SHALL be invoked with `ERROR_CODE_NETWORK_CONNECT_TIMEOUT`

#### Scenario: No timeout when zero
- **WHEN** `fun_network_tcp_connect` is called with `timeout_ms = 0`
- **THEN** the connection attempt SHALL wait indefinitely until it succeeds or fails

### Requirement: Data can be sent over a TCP connection
The system SHALL provide `fun_network_tcp_send` and `fun_network_tcp_send_vector` functions for single-buffer and scatter/gather transmission respectively.

#### Scenario: Send completes and notifies caller
- **WHEN** `fun_network_tcp_send` is called on a connected `NetworkConnection` and data is successfully transmitted
- **THEN** the `on_write_complete` handler SHALL be invoked with the number of bytes sent

#### Scenario: Send-vector transmits all segments in order
- **WHEN** `fun_network_tcp_send_vector` is called with a `NetworkBufferVector` containing segments [A, B, C]
- **THEN** the bytes SHALL be transmitted in order A then B then C, and `on_write_complete` SHALL fire once all segments are sent

#### Scenario: Send on closed connection returns error
- **WHEN** `fun_network_tcp_send` or `fun_network_tcp_send_vector` is called on a closed connection
- **THEN** the function SHALL return an error result

### Requirement: Write backpressure is signalled to caller
The system SHALL return `ERROR_CODE_NETWORK_WOULD_BLOCK` when the kernel send buffer is full. The caller MUST wait for `on_write_complete` before sending again.

#### Scenario: Send when buffer full returns would-block
- **WHEN** `fun_network_tcp_send` is called and the kernel send buffer cannot accept any bytes
- **THEN** the function SHALL return an error result with code `ERROR_CODE_NETWORK_WOULD_BLOCK`

#### Scenario: on_write_complete signals readiness to send again
- **WHEN** the kernel send buffer drains after a would-block condition
- **THEN** the `on_write_complete` handler SHALL be invoked

### Requirement: Partial writes are completed internally
When the kernel accepts fewer bytes than requested, the arch layer SHALL re-arm the write-ready event and continue sending. `on_write_complete` fires only when all bytes are sent.

#### Scenario: Partial kernel write completes fully
- **WHEN** `fun_network_tcp_send` is called with N bytes and the kernel initially accepts fewer than N
- **THEN** the module SHALL continue sending the remainder and invoke `on_write_complete` only after all N bytes are sent

### Requirement: Incoming data is delivered via scatter-receive callback
The `on_read` handler is invoked with signature `void (*on_read)(NetworkConnection *, NetworkBufferVector, int bytes_received)`. The arch layer uses `readv` / `WSARecv` with multiple buffers to scatter incoming bytes.

#### Scenario: Read callback delivers received bytes
- **WHEN** the remote end sends data
- **THEN** the `on_read` handler SHALL be called with the bytes scattered across the caller's `NetworkBufferVector` segments

### Requirement: Callbacks may call send, close, and stop re-entrantly
`fun_network_tcp_send`, `fun_network_tcp_send_vector`, `fun_network_tcp_close`, and `fun_network_loop_stop` are safe to call from within any callback. Re-entrant calls are deferred and processed after the current callback returns.

#### Scenario: Send from within on_read
- **WHEN** `fun_network_tcp_send` is called from within an `on_read` callback
- **THEN** the send SHALL be enqueued with no deadlock

#### Scenario: Close from within on_error
- **WHEN** `fun_network_tcp_close` is called from within an `on_error` callback
- **THEN** the connection SHALL be marked for close and `on_close` SHALL fire after the callback returns

### Requirement: TCP connection can be closed
The system SHALL provide `fun_network_tcp_close` for graceful connection shutdown.

#### Scenario: Close triggers on_close callback
- **WHEN** `fun_network_tcp_close` is called on a connected `NetworkConnection`
- **THEN** the connection SHALL be gracefully closed and `on_close` SHALL be invoked

#### Scenario: Remote close triggers on_close callback
- **WHEN** the remote end closes the connection
- **THEN** the `on_close` handler SHALL be invoked

---

## UDP Socket

### Requirement: UDP socket can be created and bound
The system SHALL provide `fun_network_udp_bind` to create a UDP socket, bind it to a local `NetworkAddress`, and register it with a `NetworkLoop`.

#### Scenario: Bind to available port succeeds
- **WHEN** `fun_network_udp_bind` is called with a local address on an available port
- **THEN** the socket SHALL be bound and ready to send and receive

#### Scenario: Bind to port zero gets OS-assigned port
- **WHEN** `fun_network_udp_bind` is called with port 0
- **THEN** the OS SHALL assign an available ephemeral port

#### Scenario: Bind to in-use port returns error
- **WHEN** `fun_network_udp_bind` is called with a port already in use
- **THEN** `fun_network_udp_bind` SHALL return an error result

### Requirement: Datagrams can be sent
The system SHALL provide `fun_network_udp_send_to` to send a `NetworkBuffer` as a UDP datagram to a `NetworkAddress`.

#### Scenario: Send succeeds for valid destination
- **WHEN** `fun_network_udp_send_to` is called with a valid destination address and non-empty buffer
- **THEN** the datagram SHALL be enqueued for transmission

#### Scenario: Send on unbound socket returns error
- **WHEN** `fun_network_udp_send_to` is called on an unbound connection
- **THEN** `fun_network_udp_send_to` SHALL return an error result

### Requirement: Incoming datagrams are delivered via callback
The system SHALL invoke `on_datagram_read` with signature `void (*on_datagram_read)(NetworkConnection *, NetworkBuffer, NetworkAddress sender)` when a datagram arrives.

#### Scenario: Callback delivers datagram and sender address
- **WHEN** a datagram arrives on the bound socket
- **THEN** `on_datagram_read` SHALL be invoked with the datagram bytes and the sender's `NetworkAddress`

#### Scenario: Oversized datagram is truncated to buffer size
- **WHEN** a datagram arrives that exceeds the caller's receive buffer
- **THEN** `on_datagram_read` SHALL be invoked with the bytes that fit and the excess SHALL be discarded

### Requirement: UDP socket can be closed
The system SHALL provide `fun_network_udp_close` to close the socket and deregister it from the loop.

#### Scenario: Close releases socket and triggers callback
- **WHEN** `fun_network_udp_close` is called on a bound socket
- **THEN** the socket SHALL be closed and `on_close` SHALL be invoked
