## 1. Public API Header

- [ ] 1.1 Create `include/network/network.h` — define `NetworkAddress`, `NetworkBuffer`, `NetworkBufferVector`, `NetworkHandlers` (with `void *user_data`, separate `on_read` for TCP and `on_datagram_read` for UDP), `NetworkConnection`, `NetworkLoop` opaque types with `NETWORK_LOOP_SIZE`/`NETWORK_CONNECTION_SIZE` size constants; add `fun_network_connection_get_user_data` accessor
- [ ] 1.2 Declare `fun_network_address_parse`, `fun_network_address_to_string` in header
- [ ] 1.3 Declare `fun_network_buffer_slice`, `fun_network_buffer_vector_total_length` and document `NetworkBuffer`/`NetworkBufferVector` field layout in header
- [ ] 1.4 Declare `fun_network_loop_init`, `fun_network_loop_run`, `fun_network_loop_run_once` (with `timeout_ms` param), `fun_network_loop_stop`, `fun_network_loop_destroy` in header
- [ ] 1.5 Declare `fun_network_tcp_connect` (with `timeout_ms` param), `fun_network_tcp_send`, `fun_network_tcp_send_vector`, `fun_network_tcp_close` in header; define `ERROR_CODE_NETWORK_WOULD_BLOCK` and `ERROR_CODE_NETWORK_CONNECT_TIMEOUT`
- [ ] 1.6 Declare `fun_network_udp_bind`, `fun_network_udp_send_to`, `fun_network_udp_close` in header
- [ ] 1.7 Define all `ERROR_CODE_NETWORK_*` error constants and `DEFINE_RESULT_TYPE` entries for `NetworkAddress`, `NetworkBuffer`, `NetworkConnection`

## 2. Core Implementation (src/)

- [ ] 2.1 Create `src/network/network.c` — implement `fun_network_address_parse` (IPv4 and IPv6 numeric-only parsing, port validation 0–65535, reject hostname strings)
- [ ] 2.2 Implement `fun_network_address_to_string` in `src/network/network.c`
- [ ] 2.3 Implement `fun_network_buffer_slice` and `fun_network_buffer_vector_total_length` in `src/network/network.c`
- [ ] 2.4 Implement `fun_network_loop_init` / `fun_network_loop_destroy` — call arch-layer init/destroy, no platform logic
- [ ] 2.5 Implement `fun_network_loop_run` — delegate to arch-layer blocking loop
- [ ] 2.6 Implement `fun_network_loop_run_once` — pass `timeout_ms` to arch-layer (0 = poll, -1 = block, positive = timed wait)
- [ ] 2.7 Implement `fun_network_loop_stop` — set stop flag via arch layer
- [ ] 2.8 Implement `fun_network_tcp_connect` — validate args, pass `timeout_ms` to arch-layer connect
- [ ] 2.9 Implement `fun_network_tcp_send` — validate state, return `ERROR_CODE_NETWORK_WOULD_BLOCK` when arch layer reports buffer full; defer re-entrant sends from callbacks
- [ ] 2.10 Implement `fun_network_tcp_send_vector` — validate state, pass `NetworkBufferVector` to arch-layer vectored send; same backpressure and re-entrancy contract as single send
- [ ] 2.11 Implement `fun_network_tcp_close` — call arch-layer close
- [ ] 2.12 Implement `fun_network_udp_bind` — validate args, call arch-layer bind
- [ ] 2.13 Implement `fun_network_udp_send_to` — validate state, call arch-layer send-to
- [ ] 2.14 Implement `fun_network_udp_close` — call arch-layer close

## 3. Linux Arch Layer (arch/network/linux-amd64/)

- [ ] 3.1 Create `arch/network/linux-amd64/network.c` — epoll-based event loop: `epoll_create1`, per-connection `epoll_ctl` registration
- [ ] 3.2 Implement arch loop init/destroy: create epoll fd, initialise stop flag
- [ ] 3.3 Implement arch loop run (blocking `epoll_wait` loop) and run-once (pass `timeout_ms` to `epoll_wait`)
- [ ] 3.4 Implement arch TCP connect: create non-blocking socket, `connect()`, register with epoll for `EPOLLOUT` (detect connect completion) then switch to `EPOLLIN`; track connect start time for timeout expiry
- [ ] 3.5 Implement arch TCP send: `send()` on write-ready; on partial write, track remaining bytes and re-arm `EPOLLOUT`; invoke `on_write_complete` only when all bytes sent; return would-block when buffer full; support deferred sends from within callbacks
- [ ] 3.6 Implement arch TCP send-vector: `writev()` with `struct iovec` array mapped from `NetworkBufferVector`; same partial-write and backpressure handling as single send
- [ ] 3.7 Implement arch TCP recv: `readv()` on `EPOLLIN` event with caller's `NetworkBufferVector`, invoke `on_read` callback with bytes received; handle `EPOLLERR`/`EPOLLHUP` → `on_close`
- [ ] 3.8 Implement arch TCP close: `epoll_ctl` remove, `close()` fd, invoke `on_close`
- [ ] 3.9 Implement arch UDP bind: `socket(SOCK_DGRAM)`, `bind()`, register with epoll `EPOLLIN`
- [ ] 3.10 Implement arch UDP send-to: `sendto()` non-blocking
- [ ] 3.11 Implement arch UDP recv: `recvfrom()` on `EPOLLIN`, pass sender address and data to `on_datagram_read`
- [ ] 3.12 Implement arch UDP close: epoll remove, `close()`, invoke `on_close`

## 4. Windows Arch Layer (arch/network/windows-amd64/)

- [ ] 4.1 Create `arch/network/windows-amd64/network.c` — IOCP-based implementation: `CreateIoCompletionPort`, per-connection `OVERLAPPED` bookkeeping
- [ ] 4.2 Implement arch loop init: `WSAStartup`, create IOCP handle; destroy: `CloseHandle`, `WSACleanup`
- [ ] 4.3 Implement arch loop run: `GetQueuedCompletionStatus` loop; run-once: pass `timeout_ms` to `GetQueuedCompletionStatus`
- [ ] 4.4 Implement arch TCP connect: `WSASocket`, associate with IOCP, `ConnectEx` with OVERLAPPED; on completion invoke `on_connect`; track connect start time for timeout expiry
- [ ] 4.5 Implement arch TCP send: `WSASend` with single `WSABUF` and OVERLAPPED; handle partial completions internally; invoke `on_write_complete` only when all bytes sent; support deferred sends from within callbacks
- [ ] 4.6 Implement arch TCP send-vector: `WSASend` with multiple `WSABUF`s mapped from `NetworkBufferVector`; same partial-write and backpressure handling as single send
- [ ] 4.7 Implement arch TCP recv: post `WSARecv` with multiple `WSABUF`s from caller's `NetworkBufferVector`; on completion invoke `on_read` with bytes received; on error/close invoke `on_close`
- [ ] 4.8 Implement arch TCP close: `closesocket`, invoke `on_close`
- [ ] 4.9 Implement arch UDP bind: `WSASocket(SOCK_DGRAM)`, associate with IOCP, `bind()`
- [ ] 4.10 Implement arch UDP send-to: `WSASendTo` with OVERLAPPED
- [ ] 4.11 Implement arch UDP recv: `WSARecvFrom` with OVERLAPPED; on completion invoke `on_datagram_read` with sender address

## 5. Tests

- [ ] 5.1 Create `tests/network/test_network.c` — test `fun_network_address_parse` for IPv4, IPv6, invalid inputs, port boundaries (0 accepted, >65535 rejected), hostname rejection
- [ ] 5.2 Add test for `fun_network_address_to_string` roundtrip (parse → format → compare)
- [ ] 5.3 Add test for `fun_network_buffer_slice` — valid slice, out-of-bounds slice
- [ ] 5.4 Add test for `fun_network_buffer_vector_total_length` — multiple segments, empty vector
- [ ] 5.5 Add loopback TCP test: connect to localhost echo server (or self-pipe), send data, verify `on_read` callback receives it
- [ ] 5.6 Add loopback TCP vectored send test: send via `fun_network_tcp_send_vector` with multiple `NetworkBuffer` segments, verify all bytes arrive in order
- [ ] 5.7 Add TCP scatter receive test: provide multi-segment `NetworkBufferVector` for receive, verify bytes are scattered across segments
- [ ] 5.8 Add UDP loopback test: bind two sockets, send datagram from one to the other, verify `on_datagram_read` delivers bytes and sender address
- [ ] 5.9 Add error-path tests: connect to refused port (`on_error`), connect timeout (`on_error` with `ERROR_CODE_NETWORK_CONNECT_TIMEOUT`), send on closed connection, send when buffer full (would-block), invalid address parse
- [ ] 5.10 Create `tests/network/build-windows-amd64.bat` — compile with `ws2_32` link
- [ ] 5.11 Create `tests/network/build-linux-amd64.sh` — compile without extra libs; set execute bit with `git update-index --chmod=+x`
- [ ] 5.12 Add `tests/network/` to `run-tests-windows-amd64.bat` master test runner
- [ ] 5.13 Add `tests/network/` to `run-tests-linux-amd64.sh` master test runner

## 6. Build Script Integration

- [ ] 6.1 Add `src/network/network.c`, `arch/network/windows-amd64/network.c` to `fundamental-cli/build-windows-amd64.bat` and link `ws2_32`
- [ ] 6.2 Add `src/network/network.c`, `arch/network/linux-amd64/network.c` to `fundamental-cli/build-linux-amd64.sh`
- [ ] 6.3 Run `vendor-fundamental.bat` in `fundamental-cli/` and remove any stale vendored files

## 7. Code Formatting & Specification

- [ ] 7.1 Run `code-format.bat` on all new `.c` and `.h` files
- [ ] 7.2 Create `openspec/specs/network/spec.md` — canonical merged spec (aggregate of net-loop, net-tcp-client, net-udp-socket, net-address, net-buffer specs)

## 8. Documentation & Skills

- [ ] 8.1 Create `.opencode/skills/fundamental-network.md` — Quick Reference table + copy-paste examples for loop init/run, TCP connect/send/send-vector, UDP bind/send, address parse, scatter/gather patterns
- [ ] 8.2 Add `network` row to skills index table in `.opencode/skills/fundamental-skills-index.md`
- [ ] 8.3 Add `network` row to Modules table and Skills table in `CLAUDE.md`
