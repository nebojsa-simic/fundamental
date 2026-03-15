## REMOVED Requirements

### Requirement: Event loop can be created and destroyed
**Reason**: Reactor model removed. The new simple async interface uses self-polling `AsyncResult`; no external event loop is needed.
**Migration**: Remove `NetworkLoop` storage, `fun_network_loop_init`, and `fun_network_loop_destroy` calls. Use `fun_network_tcp_connect` directly.

### Requirement: Event loop can run in blocking mode
**Reason**: Reactor model removed.
**Migration**: Replace `fun_network_loop_run` with `fun_async_await(&result, timeout_ms)`.

### Requirement: Event loop can tick with optional timeout
**Reason**: Reactor model removed.
**Migration**: Replace `fun_network_loop_run_once` with `fun_async_await(&result, timeout_ms)`.

### Requirement: Event loop can be stopped
**Reason**: Reactor model removed.
**Migration**: No equivalent needed; `fun_async_await` returns when the operation completes or times out.

### Requirement: NetworkBuffer wraps caller-owned memory
**Reason**: `NetworkBuffer` is redefined. The old definition (`void *data`, `size_t length`) is replaced by one that uses the `Memory` type and adds `frame_size`.
**Migration**: Update all `NetworkBuffer` initialisers: `{ ptr, len }` → `{ mem, len, 0 }`.

### Requirement: NetworkBuffer can be sliced
**Reason**: `fun_network_buffer_slice` is removed. Slicing was only needed for scatter/gather patterns that no longer exist in the API.
**Migration**: Compute sub-buffer offsets manually using pointer arithmetic on `Memory`.

### Requirement: NetworkBufferVector represents a scatter/gather vector of buffers
**Reason**: Scatter/gather vectored I/O is removed from the simple async API. Single-buffer send and receive cover all CLI use cases.
**Migration**: Consolidate scatter buffers into a single contiguous `NetworkBuffer` before passing to `fun_network_tcp_send`.

### Requirement: TCP connection can be initiated
**Reason**: Replaced by `fun_network_tcp_connect` with `AsyncResult` return (see network-simple-async spec).
**Migration**: Remove `NetworkLoop`, `NetworkHandlers`, and caller-allocated `NetworkConnection` storage. Use `TcpNetworkConnection conn = NULL; fun_network_tcp_connect(addr, &conn)`.

### Requirement: TCP connect supports a timeout
**Reason**: Timeout is now controlled by `fun_async_await(&result, timeout_ms)` at the call site.
**Migration**: Pass the desired timeout to `fun_async_await` instead of `fun_network_tcp_connect`.

### Requirement: Data can be sent over a TCP connection
**Reason**: Replaced by `fun_network_tcp_send` (see network-simple-async spec). Scatter/gather `fun_network_tcp_send_vector` is removed.
**Migration**: Use `fun_network_tcp_send(conn, request)` and await the result.

### Requirement: Write backpressure is signalled to caller
**Reason**: The new `fun_network_tcp_send` blocks internally until all bytes are accepted; `ERROR_CODE_NETWORK_WOULD_BLOCK` is removed.
**Migration**: No action required. Backpressure is handled transparently.

### Requirement: Partial writes are completed internally
**Reason**: Subsumed into the new `fun_network_tcp_send` contract, which does not complete until all bytes are written.
**Migration**: No action required.

### Requirement: Incoming data is delivered via scatter-receive callback
**Reason**: Callback-based receive removed. Use `fun_network_tcp_receive_exact` or `fun_network_tcp_receive_until`.
**Migration**: Remove `on_read` handler. Call `fun_network_tcp_receive_exact` or `fun_network_tcp_receive_until` and await the result.

### Requirement: Callbacks may call send, close, and stop re-entrantly
**Reason**: No callbacks exist in the new API.
**Migration**: Not applicable.

### Requirement: TCP connection can be closed
**Reason**: Replaced by `fun_network_tcp_close(conn)` (see network-simple-async spec). No `on_close` callback.
**Migration**: Call `fun_network_tcp_close(conn)` directly when done.

### Requirement: UDP socket can be created and bound
**Reason**: Explicit bind/listen UDP API removed. UDP is fire-and-forget send only via `fun_network_udp_send`.
**Migration**: Use `fun_network_udp_send(address, datagram)`.

### Requirement: Datagrams can be sent
**Reason**: `fun_network_udp_send_to` removed. Replaced by `fun_network_udp_send`.
**Migration**: Use `fun_network_udp_send(address, datagram)`.

### Requirement: Incoming datagrams are delivered via callback
**Reason**: UDP receive removed from this module.
**Migration**: Not available in this API version.

### Requirement: UDP socket can be closed
**Reason**: Ephemeral socket is created and closed internally by `fun_network_udp_send`; no explicit close needed.
**Migration**: Remove `fun_network_udp_close` calls.

## MODIFIED Requirements

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
