## Why

The current network module exposes a reactor-based event loop (`NetworkLoop`, `NetworkHandlers`, callbacks) that is powerful but too complex for the primary use case: CLI tools making a small number of outbound TCP/UDP requests. The callback inversion-of-control model forces callers to split straightforward request/response logic across multiple functions, making code hard to follow and error-prone. A simple async interface — connect, send, receive, close — fits CLI workloads and composes naturally with the existing `AsyncResult` infrastructure.

## What Changes

- **BREAKING** Remove `NetworkLoop`, `NetworkHandlers`, `NetworkBufferVector`, `NetworkConnection`, and all reactor-lifecycle functions (`fun_network_loop_init`, `fun_network_loop_run`, `fun_network_loop_run_once`, `fun_network_loop_stop`, `fun_network_loop_destroy`)
- **BREAKING** Remove `fun_network_tcp_send`, `fun_network_tcp_send_vector`, `fun_network_tcp_close` (old signatures), `fun_network_udp_bind`, `fun_network_udp_send_to`, `fun_network_udp_close`, `fun_network_connection_get_user_data`
- Add `TcpNetworkConnection` opaque handle type (pointer typedef, managed by internal per-thread reactor)
- Add `OutputTcpNetworkConnection` and `OutputNetworkBuffer` output-parameter typedefs
- Add `fun_network_tcp_connect(address, &conn)` — async, returns `AsyncResult`
- Add `fun_network_tcp_send(conn, request)` — async send
- Add `fun_network_tcp_receive_exact(conn, &response, bytes)` — async receive of exactly N bytes; overflow bytes from the socket are buffered internally on the connection and consumed by the next receive call
- Add `fun_network_tcp_close(conn)` — synchronous, returns connection to reactor pool
- Add `fun_network_udp_send(address, datagram)` — fire-and-forget async UDP send
- Retain `fun_network_address_parse` and `fun_network_address_to_string` unchanged

## Capabilities

### New Capabilities

- `network-simple-async`: Simple async TCP/UDP interface — connect, send, receive-exact, close — built on self-polling `AsyncResult` with internal per-connection overflow buffering

### Modified Capabilities

- `network`: Existing network capability spec is replaced in full; the reactor-based model is removed and the simple async model is the new normative surface

## Impact

- `include/network/network.h` — full rewrite
- `src/network/network.c` — full rewrite
- `arch/network/windows-amd64/network.c` — full rewrite (IOCP reactor replaced by WSAPoll-based self-polling)
- `arch/network/linux-amd64/network.c` — full rewrite (epoll reactor replaced by poll-based self-polling)
- `tests/network/` — full rewrite
- `openspec/specs/network/spec.md` — replaced
- `.opencode/skills/fundamental-network.md` — updated to new API surface
- No dependencies on the network module exist in other modules; no transitive impact
