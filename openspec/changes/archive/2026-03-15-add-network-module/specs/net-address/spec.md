## ADDED Requirements

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
