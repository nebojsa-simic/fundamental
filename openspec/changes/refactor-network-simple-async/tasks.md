## 1. Header

- [ ] 1.1 Rewrite `include/network/network.h`: remove all reactor types and functions; define `NetworkBuffer`, `TcpNetworkConnection`, `OutputTcpNetworkConnection`, `OutputNetworkBuffer`, error codes, and all new function declarations

## 2. Core Implementation

- [ ] 2.1 Implement internal per-thread reactor: connection pool, overflow buffer allocation, config-module integration for `rx_buf_size` default (`src/network/network.c`)
- [ ] 2.2 Implement `fun_network_tcp_connect`: initiate non-blocking connect, allocate connection slot, return self-polling `AsyncResult`
- [ ] 2.3 Implement `fun_network_tcp_send`: enqueue send, return `AsyncResult` that polls until all bytes written
- [ ] 2.4 Implement `fun_network_tcp_receive_exact`: drain overflow buffer then socket until exactly N bytes received; store surplus bytes in overflow buffer
- [ ] 2.5 Implement `fun_network_tcp_close`: close socket, free overflow buffer, return slot to pool; NULL-safe
- [ ] 2.6 Implement `fun_network_udp_send`: create ephemeral socket, send datagram, close socket, return `AsyncResult`
- [ ] 2.7 Retain `fun_network_address_parse` and `fun_network_address_to_string` (no functional change; update includes if needed)

## 3. Windows Arch Layer

- [ ] 3.1 Rewrite `arch/network/windows-amd64/network.c`: replace IOCP reactor with `WSAPoll`-based self-polling; implement all platform socket operations called by core (`connect`, `send`, `recv`, `close`, `udp_send`)

## 4. Linux Arch Layer

- [ ] 4.1 Rewrite `arch/network/linux-amd64/network.c`: replace epoll reactor with `poll`-based self-polling; implement all platform socket operations called by core

## 5. Tests

- [ ] 5.1 Rewrite `tests/network/test_network.c`: remove all reactor-based tests
- [ ] 5.2 Add test: `fun_network_tcp_connect` success and unreachable-host error
- [ ] 5.3 Add test: `fun_network_tcp_send` + `fun_network_tcp_receive_exact` round-trip (loopback)
- [ ] 5.4 Add test: `fun_network_tcp_receive_exact` — surplus bytes stored in overflow buffer and consumed by next receive
- [ ] 5.5 Add test: `fun_network_udp_send` completes without error
- [ ] 5.6 Update `tests/network/build-windows-amd64.bat` and `tests/network/build-linux-amd64.sh`

## 6. Specs and Skills

- [ ] 6.1 Copy `openspec/changes/refactor-network-simple-async/specs/network-simple-async/spec.md` → `openspec/specs/network-simple-async/spec.md`
- [ ] 6.2 Apply delta from `openspec/changes/refactor-network-simple-async/specs/network/spec.md` to `openspec/specs/network/spec.md`
- [ ] 6.3 Rewrite `.opencode/skills/fundamental-network.md` with new API surface, quick-reference table, and copy-paste examples for TCP connect/send/receive-exact and UDP send
- [ ] 6.4 Update `.opencode/skills/fundamental-skills-index.md` entry for network module
- [ ] 6.5 Update `CLAUDE.md` network module row (Status: Complete, Key Functions updated)

## 7. Formatting and Build Verification

- [ ] 7.1 Run `code-format.bat` across all modified `.c` and `.h` files
- [ ] 7.2 Build and run `tests/network/` on Windows; confirm all tests pass
