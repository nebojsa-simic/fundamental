## Why

The existing network module provides TCP/UDP client capabilities (connect, send, receive) but lacks server-side functionality. CLI applications that need to accept incoming connections or receive UDP datagrams currently have no simple, async-native way to listen. This change adds a minimal two-stage server API that matches the library's async pattern and caller-allocated philosophy.

## What Changes

- Add `NetworkServerConfig` opaque type for server configuration
- Add `fun_network_tcp_server_config()` and `fun_network_udp_server_config()` for the config stage
- Add `fun_network_tcp_listen()` and `fun_network_udp_listen()` for the listen stage, each returning an `AsyncResult`
- Internal accept/receive loop running on a background thread
- Server shutdown via `fun_network_server_stop()` and `fun_network_server_config_free()`
- Demo: simple in-memory message broker (TCP PUB/SUB)
- Add `fun_network_server_get_port()` for port discovery after bind (essential for ephemeral ports)
- SO_REUSEADDR enabled by default on server sockets for fast restart
- TCP_NODELAY enabled by default on accepted connections for low-latency messaging
- Server config type safety: TCP/UDP listen functions validate config type at runtime
- EINTR handling: accept/receive loops retry on signal interruption
- Stop-during-callback: blocks until current callback returns; safe to call `fun_network_server_stop()` from within a callback

## Capabilities

### New Capabilities
- `network-server`: Async TCP/UDP server with two-stage API (config + listen). Caller-allocated buffers, caller-managed state passed through to listener callbacks. `AsyncResult` represents server lifetime — PENDING while running, ERROR on bind failure, COMPLETED after stop.
- `network-server-port-discovery`: Query the actual bound port after listen, supporting ephemeral port (port 0) allocation.

### Modified Capabilities
- None

## Impact

- New header: `include/fundamental/network/server.h`
- New source files: `src/network/server/`
- Platform-specific implementations: `arch/network/server/windows-amd64/`, `arch/network/server/linux-amd64/`
- Extends existing network module; no breaking changes to client APIs
- New test directory: `tests/network-server/`
- New demo directory: `demos/network-server/`
