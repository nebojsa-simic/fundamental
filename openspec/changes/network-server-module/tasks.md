## 1. Header and Module Structure

- [ ] 1.1 Create `include/fundamental/network/server.h` with all function declarations, type definitions, and callback signatures
- [ ] 1.2 Create `src/network/server/` directory and `server.c` with shared implementation stubs
- [ ] 1.3 Create `arch/network/server/windows-amd64/` directory with `server.c` for Windows platform
- [ ] 1.4 Create `arch/network/server/linux-amd64/` directory with `server.c` for Linux platform

## 2. TCP Server Config API

- [ ] 2.1 Implement `fun_network_tcp_server_config()` — validate address, allocate internal config, store address and server_state
- [ ] 2.2 Implement `fun_network_server_config_free()` — free internal config resources
- [ ] 2.3 Add error handling for NULL pointers and invalid inputs

## 3. UDP Server Config API

- [ ] 3.1 Implement `fun_network_udp_server_config()` — validate address, buffer, buffer_size; allocate internal config
- [ ] 3.2 Store buffer pointer and size in config for receive loop use
- [ ] 3.3 Add error handling for NULL buffer and zero buffer_size

## 4. Windows TCP Server Implementation

- [ ] 4.1 Implement `fun_network_tcp_listen()` — create listen socket, bind, call listen(), spawn accept thread
- [ ] 4.2 Implement accept loop — `accept()` in loop, invoke `NetworkTcpListener` callback per connection
- [ ] 4.3 Implement `fun_network_server_stop()` — set stop flag, close listen socket, join thread
- [ ] 4.4 Wrap accepted socket into `TcpNetworkConnection` handle compatible with existing TCP APIs

## 5. Windows UDP Server Implementation

- [ ] 5.1 Implement `fun_network_udp_listen()` — create UDP socket, bind, spawn receive thread
- [ ] 5.2 Implement receive loop — `recvfrom()` in loop, parse source address, invoke `NetworkUdpListener` callback
- [ ] 5.3 Integrate stop signal handling for UDP receive thread

## 6. Linux TCP Server Implementation

- [ ] 6.1 Implement `fun_network_tcp_listen()` — create listen socket, bind, call listen(), spawn accept thread
- [ ] 6.2 Implement accept loop — `accept()` in loop, invoke `NetworkTcpListener` callback per connection
- [ ] 6.3 Implement `fun_network_server_stop()` — set stop flag, close listen socket, join thread
- [ ] 6.4 Wrap accepted socket into `TcpNetworkConnection` handle compatible with existing TCP APIs

## 7. Linux UDP Server Implementation

- [ ] 7.1 Implement `fun_network_udp_listen()` — create UDP socket, bind, spawn receive thread
- [ ] 7.2 Implement receive loop — `recvfrom()` in loop, parse source address, invoke `NetworkUdpListener` callback
- [ ] 7.3 Integrate stop signal handling for UDP receive thread

## 8. Build Scripts

- [ ] 8.1 Create `tests/network-server/build-windows-amd64.bat` with all source files and `-lws2_32`
- [ ] 8.2 Create `tests/network-server/build-linux-amd64.sh` with all source files and `-lpthread`

## 9. Tests

- [ ] 9.1 Test TCP config creation with valid and invalid addresses
- [ ] 9.2 Test UDP config creation with valid/NULL buffer and valid/zero buffer_size
- [ ] 9.3 Test TCP listen bind failure (port already in use) returns ASYNC_ERROR
- [ ] 9.4 Test UDP listen bind failure returns ASYNC_ERROR
- [ ] 9.5 Test TCP listen success returns ASYNC_PENDING
- [ ] 9.6 Test TCP callback invoked on client connection with correct server_state
- [ ] 9.7 Test UDP callback invoked on datagram receipt with correct source and data
- [ ] 9.8 Test server stop transitions result to ASYNC_COMPLETED
- [ ] 9.9 Test config free after stop releases resources (no memory leak)
- [ ] 9.10 Test TCP client handle works with existing send/receive/close APIs

## 10. Demo — In-Memory Message Broker

- [ ] 10.1 Create `demos/network-server/` directory
- [ ] 10.2 Implement `demo.c` — TCP message broker with PUB/SUB/QUIT commands
- [ ] 10.3 Create `demos/network-server/build-windows-amd64.bat`
- [ ] 10.4 Create `demos/network-server/build-linux-amd64.sh`
- [ ] 10.5 Test demo on Windows — connect with netcat, send PUB/SUB commands
- [ ] 10.6 Test demo on Linux — connect with netcat, send PUB/SUB commands

## 11. Port Discovery

- [ ] 11.1 Implement `fun_network_server_get_port()` — query bound socket via `getsockname()`, extract port in host byte order
- [ ] 11.2 Test port discovery after TCP listen on port 0 (ephemeral) returns valid port
- [ ] 11.3 Test port discovery after UDP listen on port 0 returns valid port
- [ ] 11.4 Test port discovery on fixed non-zero port returns the correct port

## 12. SO_REUSEADDR

- [ ] 12.1 Set SO_REUSEADDR on TCP server socket before bind (Windows + Linux)
- [ ] 12.2 Set SO_REUSEADDR on UDP server socket before bind (Windows + Linux)
- [ ] 12.3 Test rapid stop/restart on same port succeeds without TIME_WAIT delay

## 13. TCP_NODELAY on Accepted Connections

- [ ] 13.1 Set TCP_NODELAY on accepted client socket before invoking callback (Windows)
- [ ] 13.2 Set TCP_NODELAY on accepted client socket before invoking callback (Linux)

## 14. Config Type Validation

- [ ] 14.1 Add `server_type` tag field to internal config struct (`NETWORK_SERVER_TCP` / `NETWORK_SERVER_UDP`)
- [ ] 14.2 `fun_network_tcp_listen()` validates config type before any socket operation
- [ ] 14.3 `fun_network_udp_listen()` validates config type before any socket operation
- [ ] 14.4 Define `ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE` error constant
- [ ] 14.5 Test TCP listen with UDP config returns ASYNC_ERROR
- [ ] 14.6 Test UDP listen with TCP config returns ASYNC_ERROR

## 15. EINTR Handling

- [ ] 15.1 Linux TCP accept loop: retry on EINTR
- [ ] 15.2 Linux UDP recvfrom loop: retry on EINTR

## 16. Stop-During-Callback Semantics

- [ ] 16.1 Document stop() blocks until callback returns in header comments
- [ ] 16.2 Test stop() called while callback is sleeping blocks until callback returns
- [ ] 16.3 Test stop() called from within callback exits cleanly without deadlock

## 17. Missing Test Coverage

- [ ] 17.1 Test config free before stop (abuse case: use-after-free validation)
- [ ] 17.2 Load test: 100+ concurrent TCP connections accepted without drops
- [ ] 17.3 Test UDP datagram truncation: oversized datagram is truncated to buffer_size
- [ ] 17.4 Test NULL callback passed to listen returns error
- [ ] 17.5 Test rapid stop/start cycles on same port (covered by SO_REUSEADDR tests)
- [ ] 17.6 Test accept loop continues while callback blocks (verify bottleneck behavior)
- [ ] 17.7 Test accept() failure handling (simulate EMFILE by exhausting fd limit)

## 18. Validation

- [ ] 18.1 Run `run-tests-windows-amd64.bat` — all network-server tests pass
- [ ] 18.2 Run `./run-tests-linux-amd64.sh` — all network-server tests pass
- [ ] 18.3 Run `code-format.bat` — clang-format passes on all new files
- [ ] 18.4 Run `openspec validate network-server-module` — all specs validated
