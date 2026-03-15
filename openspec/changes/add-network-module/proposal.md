## Why

The Fundamental Library has no way to communicate over a network. Applications built on it cannot connect to servers, implement clients, or exchange data over TCP/UDP sockets. A non-blocking, event-driven network module — inspired by Netty, Boost.Asio, Java NIO, golang gnet, and Rust Tokio — fills this gap while staying true to the library's zero-stdlib, caller-allocated design.

## What Changes

- Add `network` module: `include/network/network.h`, `src/network/network.c`, `arch/network/windows-amd64/network.c`, `arch/network/linux-amd64/network.c`
- Introduce a reactor/event-loop model: a single `NetworkLoop` drives all I/O via platform-native multiplexing (IOCP on Windows, `epoll` on Linux)
- TCP client connections: connect, send, send-vector (scatter/gather), receive, close
- UDP socket support: bind, send-to, receive-from
- Non-blocking by default; all I/O is driven by callbacks registered on the loop
- Caller-allocated connection and buffer types; no internal heap allocation for the data path
- Scatter/gather vectored I/O via `NetworkBufferVector` for zero-copy multi-segment sends and receives
- Add test suite: `tests/network/`
- Add OpenSpec spec: `openspec/specs/network/spec.md`
- Add agent skill file: `.opencode/skills/fundamental-network.md`

## Capabilities

### New Capabilities

- `net-loop`: Event loop lifecycle — create, run (blocking), run-once (non-blocking tick with timeout), stop, destroy
- `net-tcp-client`: Establish outbound TCP connections with timeout, register read/write/close handlers, send data (single and vectored), backpressure signalling
- `net-udp-socket`: Bind a UDP socket, send datagrams, receive datagrams via callback with sender address
- `net-address`: Parse and represent `host:port` addresses (IPv4, IPv6 numeric only)
- `net-buffer`: Caller-owned buffer type and scatter/gather `NetworkBufferVector` used for all read/write operations

### Modified Capabilities

<!-- none -->

## Impact

- New files only; no existing modules are modified
- Arch layer uses `winsock2.h` + IOCP on Windows and POSIX sockets + `epoll` on Linux
- Consumers must link against `ws2_32` on Windows (added to build scripts)
- The `async` module's `AsyncResult` pattern is reused conceptually; the network module does not depend on it at runtime
