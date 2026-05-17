## Context

The network module (`include/fundamental/network/network.h`) already provides async TCP client (connect, send, receive_exact, close) and UDP fire-and-forget send. All operations return `AsyncResult` and follow the library's async pattern. The module uses platform-specific implementations in `arch/network/*/` and caller-allocated memory throughout.

Server-side functionality is missing. Existing specs (`network`, `network-simple-async`) cover client operations only.

## Goals / Non-Goals

**Goals:**
- Two-stage server API: config stage (bind address + state) → listen stage (start accepting)
- TCP server: accept connections, invoke callback per client with `TcpNetworkConnection` handle
- UDP server: receive datagrams, invoke callback per datagram with source address and buffer
- Caller-allocated buffers everywhere — library never allocates user-facing memory
- `server_state` (opaque `Memory`) passed at config time, threaded through to every callback
- `AsyncResult` represents server lifetime: PENDING while running, ERROR on bind failure, COMPLETED after stop
- Internal background thread for accept/receive loop
- SO_REUSEADDR set on all server sockets for fast restart
- TCP_NODELAY set on accepted client connections for low-latency messaging
- Port discovery via `fun_network_server_get_port()` after successful listen
- EINTR resilience in accept/receive loops on Linux
- TCP listener callback receives connection and state; caller manages connection lifecycle
- Config type validation: listen functions reject mismatched configs at runtime
- Documented stop-during-callback semantics (blocks, safe from within callback)
- Graceful shutdown: `fun_network_server_stop()` signals thread exit, await confirms completion
- Demo: in-memory message broker showing real-world usage

**Non-Goals:**
- TLS/SSL support
- HTTP parsing or protocol handling
- Connection pooling or client lifecycle management
- Hostname resolution (addresses only, consistent with existing module)
- Backlog configuration (defaults to 128, configurable later via `fun.ini` if needed)
- Multiple simultaneous listeners on one config

## Decisions

### 1. Separate TCP and UDP APIs
TCP and UDP listener callbacks have fundamentally incompatible signatures. TCP hands off a connection handle; UDP hands off a datagram buffer + source address. Conflating them would require unions or type tags. Separate function names (`fun_network_tcp_server_config` / `fun_network_udp_server_config`, `fun_network_tcp_listen` / `fun_network_udp_listen`) keep the API clean.

**Alternatives considered:**
- Single `fun_network_server_config()` with a protocol enum → would need different callback signatures anyway
- Generic `void *` callback with user casting → loses type safety

### 2. `server_state` passed at config time, not via setter
Eliminates the need for `set_user_data()` setter functions. One call to create config with everything needed: address + state. Simpler API surface, fewer error paths.

### 3. UDP buffer provided at config time
UDP datagrams arrive all at once with unknown size. The caller provides a buffer at config time that the internal receive loop uses. Datagram is truncated if it exceeds buffer size. The buffer pointer is passed to the callback via `NetworkBuffer.data`. Buffer is reused after callback returns — caller must not retain references.

**Alternatives considered:**
- Library allocates per-datagram buffer → breaks caller-allocated philosophy
- Caller provides buffer pool → more complex, callback can't return a buffer to use

### 4. `AsyncResult` lifetime = server lifetime
The `AsyncResult` from `fun_network_*_listen()` is PENDING while the server is running. This allows the caller to `fun_async_await(&result, -1)` at shutdown to confirm the accept thread has exited before freeing resources. Bind failures are detected immediately by checking `result.status == ASYNC_ERROR` — no await needed for startup.

**Alternatives considered:**
- AsyncResult completes on startup → would need a separate "wait for shutdown" mechanism
- Return server handle instead → adds another opaque type, more API surface

### 5. Internal background thread for accept/receive loop
The listen function spawns a thread that runs the accept/receive loop. This keeps the API simple — caller doesn't manage threads for the server itself. Per-client handling is the caller's responsibility (spawn thread in callback, enqueue work, or handle synchronously).

**Alternatives considered:**
- Caller provides event loop → adds complexity, doesn't match library's self-polling async model
- Non-blocking accept with polling → wasteful, complex to implement correctly across platforms

### 6. Shared config type, separate listen functions
`NetworkServerConfig` is a single opaque type used by both TCP and UDP. The listen functions are type-specific because the callback signatures differ. Shutdown functions (`fun_network_server_stop`, `fun_network_server_config_free`) are shared.

### 7. Port discovery via `fun_network_server_get_port()`
After a successful listen, the caller may need to know which port the OS assigned — essential when binding to port 0 (ephemeral). The function queries the bound socket's local address and returns the port in host byte order. Must be called after listen succeeds; behavior before listen is undefined.

**Alternatives considered:**
- Store port in config automatically after bind — adds hidden state mutation; caller may not need it
- Return port from listen as output parameter — complicates listen signature

### 8. SO_REUSEADDR enabled by default
Set `SO_REUSEADDR` on all server sockets before bind. This allows fast restart without waiting for TIME_WAIT to expire and permits multiple listeners on the same port (with OS-specific semantics). Consistent with developer expectations for server software.

**Alternatives considered:**
- Configurable flag — adds API surface; SO_REUSEADDR is almost always desired for servers
- Platform-specific opt-in — would surprise users when a demo works on Linux but not Windows

### 9. TCP_NODELAY enabled by default on accepted connections
Set `TCP_NODELAY` on every accepted client socket. This disables Nagle's algorithm, ensuring small messages (like the message broker's PUB/SUB commands) are sent immediately rather than being buffered for up to 200ms. Critical for interactive and command-based protocols.

**Alternatives considered:**
- Configurable flag — unnecessary complexity; Nagle is rarely desired for application protocols
- Caller sets it manually — requires platform-specific socket option code in every callback

### 10. Config type validation
`NetworkServerConfig` stores an internal type tag (`NETWORK_SERVER_TCP` or `NETWORK_SERVER_UDP`) set at config creation. `fun_network_tcp_listen()` validates the config is TCP before proceeding; `fun_network_udp_listen()` validates it is UDP. Mismatch returns `ASYNC_ERROR` with `ERROR_CODE_NETWORK_SERVER_WRONG_CONFIG_TYPE`. Prevents silent undefined behavior at the socket layer.

**Alternatives considered:**
- Separate opaque types for TCP and UDP config — doubles the type count; shared free/stop functions would need overloads
- No validation (trust caller) — silent failures when socket ops fail on wrong protocol

### 11. EINTR handling in accept/receive loops
On Linux, `accept()` and `recvfrom()` may return `-1` with `errno = EINTR` when a signal is delivered to the thread. The accept/receive loop MUST retry the operation in this case rather than treating it as a fatal error. This is standard POSIX server behavior and is explicitly implemented in the platform-specific `arch/network/server/linux-amd64/server.c`. Windows does not have EINTR semantics.

**Alternatives considered:**
- Block signals on the accept thread — masks signals globally, may interfere with shutdown signaling
- Use `accept4()` with `SOCK_CLOEXEC` — still returns EINTR; doesn't solve the problem

### 12. Stop-during-callback semantics
When `fun_network_server_stop()` is called while a callback is executing on the internal thread:
1. `stop()` blocks until the callback returns.
2. Setting the stop flag from within a callback is safe — the thread exits after callback returns without deadlock.
3. New connections may accumulate in the backlog queue during callback execution; they are accepted and immediately closed after stop is requested.

This is documented explicitly to prevent deadlocks when callbacks call stop and to clarify that stop is blocking.

**Alternatives considered:**
- Async stop (sets flag, returns immediately) — caller would need separate await; complicates shutdown
- Callback cannot call stop — restrictive; common pattern for servers that detect shutdown conditions in callback

## Risks / Trade-offs

| Risk | Mitigation |
|---|---|
| Callback blocks accept thread | Document that callback should hand off quickly. Caller decides concurrency model. |
| UDP buffer too small → truncation | Document truncation behavior. Caller chooses buffer size based on expected datagrams. |
| Thread safety of `server_state` | Caller's responsibility. Document that callbacks may run concurrently. |
| Platform differences in socket behavior | Platform-specific implementations in `arch/network/server/windows-amd64/` and `arch/network/server/linux-amd64/` isolate differences. |
| Memory leak if caller doesn't close client connections | Document clearly that caller owns client lifecycle. Demo shows proper close pattern. |
| EINTR loop may spin on high-frequency signals | Accept loop retries on EINTR. Caller should not deliver high-frequency signals to server thread. |
| Config type mismatch caught late at listen | Type tag validated at listen entry; error returned before any socket operation. Same cost as a NULL check. |
