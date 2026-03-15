## ADDED Requirements

### Requirement: UDP socket can be created and bound
The system SHALL provide a `fun_network_udp_bind` function that creates a UDP socket, binds it to a local `NetworkAddress`, and registers it with a `NetworkLoop`.

#### Scenario: Bind to available port succeeds
- **WHEN** `fun_network_udp_bind` is called with a local address on an available port
- **THEN** the socket SHALL be bound and registered with the loop, ready to send and receive

#### Scenario: Bind to in-use port returns error
- **WHEN** `fun_network_udp_bind` is called with a port already in use
- **THEN** `fun_network_udp_bind` SHALL return an error result

#### Scenario: Bind to port zero gets OS-assigned port
- **WHEN** `fun_network_udp_bind` is called with port 0
- **THEN** the OS SHALL assign an available ephemeral port and the socket SHALL be successfully bound

### Requirement: Datagrams can be sent
The system SHALL provide a `fun_network_udp_send_to` function that sends a `NetworkBuffer` as a UDP datagram to a specified `NetworkAddress`.

#### Scenario: Send succeeds for reachable address
- **WHEN** `fun_network_udp_send_to` is called with a valid destination address and a non-empty buffer
- **THEN** the datagram SHALL be enqueued for transmission

#### Scenario: Send on unbound socket returns error
- **WHEN** `fun_network_udp_send_to` is called on a `NetworkConnection` that has not been successfully bound
- **THEN** `fun_network_udp_send_to` SHALL return an error result

### Requirement: Incoming datagrams are delivered via callback
The system SHALL invoke the registered `on_datagram_read` handler (signature: `void (*on_datagram_read)(NetworkConnection *connection, NetworkBuffer buffer, NetworkAddress sender)`) whenever a datagram arrives on a bound UDP socket. This is a separate handler from the TCP `on_read` to accommodate the sender address parameter.

#### Scenario: Read callback delivers datagram and sender address
- **WHEN** a datagram arrives on the bound socket
- **THEN** the `on_datagram_read` handler SHALL be invoked with a `NetworkBuffer` containing the datagram bytes and a `NetworkAddress` identifying the sender

#### Scenario: Oversized datagram is truncated to buffer size
- **WHEN** a datagram arrives that exceeds the caller's receive buffer size
- **THEN** the `on_datagram_read` handler SHALL be invoked with the bytes that fit in the buffer and the excess bytes SHALL be discarded

### Requirement: UDP socket can be closed
The system SHALL provide a `fun_network_udp_close` function that closes the socket and deregisters it from the loop.

#### Scenario: Close releases socket and triggers callback
- **WHEN** `fun_network_udp_close` is called on a bound socket
- **THEN** the socket SHALL be closed, deregistered from the loop, and the `on_close` handler SHALL be invoked
