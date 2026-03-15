## ADDED Requirements

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

### Requirement: NetworkBuffer reports its length
The system SHALL expose the length of a `NetworkBuffer` directly as a field, readable without a function call.

#### Scenario: Length field reflects construction value
- **WHEN** a `NetworkBuffer` is constructed with length N
- **THEN** `buffer.length` SHALL equal N

### Requirement: NetworkBufferVector represents a scatter/gather vector of buffers
The system SHALL define a `NetworkBufferVector` type that holds a pointer to a caller-owned array of `NetworkBuffer` elements and a count. This enables vectored I/O (scatter/gather) across non-contiguous memory regions in a single syscall.

#### Scenario: NetworkBufferVector can be constructed from an array of NetworkBuffers
- **WHEN** a caller initialises a `NetworkBufferVector` with a pointer to an array of `NetworkBuffer` and a count
- **THEN** the `NetworkBufferVector` SHALL refer to that array and report the correct count

#### Scenario: NetworkBufferVector with count zero is valid as empty
- **WHEN** a `NetworkBufferVector` is initialised with count 0
- **THEN** the buffer vector SHALL be considered empty and passing it to send functions SHALL have no effect

### Requirement: NetworkBufferVector total length can be computed
The system SHALL provide a `fun_network_buffer_vector_total_length` function that returns the sum of all `NetworkBuffer` lengths in the vector.

#### Scenario: Total length sums all segments
- **WHEN** `fun_network_buffer_vector_total_length` is called on a `NetworkBufferVector` with 3 buffers of lengths 100, 200, 300
- **THEN** the function SHALL return 600
