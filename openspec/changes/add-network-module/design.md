## Context

The Fundamental Library is a zero-stdlib C library. It already has an `async` module (event polling, process spawn) and a `stream` module (sequential I/O). This design introduces a `network` module that provides non-blocking TCP and UDP networking, driven by a caller-owned event loop, with no heap allocation on the hot path. The model is inspired by Netty's pipeline/handler pattern, Boost.Asio's proactor model, Go's `gnet` reactor, and Tokio's task-per-connection abstraction — distilled into idiomatic Fundamental C.

## Goals / Non-Goals

**Goals:**
- Non-blocking TCP client connections with per-connection read/write/close callbacks
- UDP send/receive via the same event loop
- Single-threaded reactor loop (epoll on Linux, IOCP on Windows)
- All memory caller-allocated; no internal heap use on the data path
- IPv4 and IPv6 address support
- Scatter/gather vectored I/O for zero-copy multi-segment sends and receives
- Composable: network module does not depend on async, stream, or file modules

**Non-Goals:**
- TCP server (accept/listen) — future change
- TLS/SSL
- Multi-threaded or work-stealing loop
- DNS resolution
- HTTP or any protocol layer
- Unix domain sockets (deferred)

## Decisions

### D1 — Reactor (readiness) on Linux, Proactor (completion) on Windows

**Decision:** Use `epoll` (readiness notification) on Linux and IOCP (completion notification) on Windows, both abstracted behind the same arch-layer API.

**Rationale:** These are the highest-performance APIs on each platform. The arch layer translates the proactor model (IOCP delivers completed I/O) and the reactor model (epoll delivers readiness, then we do the I/O) into a common callback contract: `on_read(connection, buffers, bytes_received)`, `on_write_complete(connection)`, `on_close(connection)`.

**Alternatives considered:**
- `select`/`WSAAsyncSelect`: portable but O(n) and limited to 64 sockets on Windows. Rejected.
- `kqueue` (macOS): out of scope; not a target platform.
- Unified reactor everywhere via non-blocking sockets + `select`: simpler but sacrifices IOCP performance on Windows. Rejected.

### D2 — Caller-allocated `NetworkConnection` and `NetworkLoop`

**Decision:** `NetworkLoop` and `NetworkConnection` are opaque structs whose sizes are published in the header via `NETWORK_LOOP_SIZE` and `NETWORK_CONNECTION_SIZE` constants. Callers declare storage (`char loop_buf[NETWORK_LOOP_SIZE]`) and pass a pointer; the module initialises it in-place.

**Rationale:** Consistent with the library's memory model. No hidden allocation. Caller controls lifetime and can place structs on stack or in a fixed arena.

**Alternatives considered:**
- Returning an opaque pointer from an init function (requires internal alloc). Rejected.
- Fully transparent structs in the header (exposes platform internals, breaks ABI on arch change). Rejected.

### D3 — Scatter/gather `NetworkBufferVector` for vectored I/O

**Decision:** Introduce `NetworkBuffer` (pointer + length) for single-segment operations and `NetworkBufferVector` (array of `NetworkBuffer` + count) for vectored I/O. All send and receive operations have vectored variants that map directly to `writev`/`readv` (Linux) and multi-`WSABUF` `WSASend`/`WSARecv` (Windows). Callers own all underlying memory. The module never copies data internally.

**Rationale:** Performance is a first-class citizen. Vectored I/O avoids copying non-contiguous data into a single buffer before sending — the kernel handles the gather internally. This is critical for protocol implementations that prepend headers to payload bodies. `readv`/`writev` and `WSASend`/`WSARecv` with multiple buffers are zero-cost on both platforms since the syscalls accept scatter/gather natively.

**Alternatives considered:**
- Single-buffer only (simpler API, forces caller to copy into contiguous buffer before sending): rejected — unnecessary copy on every multi-segment send.
- Internal scatter/gather that hides vectored I/O behind a single-buffer API: rejected — hides the performance characteristic and prevents callers from optimising layout.

### D4 — Callback-per-event, not virtual-dispatch pipeline

**Decision:** Each `NetworkConnection` carries a small `NetworkHandlers` struct with function pointers: `on_connect`, `on_read`, `on_write_complete`, `on_close`, `on_error`, `on_datagram_read`, plus a `void *user_data` pointer. No pipeline/chain mechanism.

**Rationale:** A pipeline (Netty-style) adds flexibility but also complexity and indirection. At this layer, direct function pointers suffice. A pipeline can be built on top by the caller. The `user_data` pointer gives callbacks access to per-connection application state (similar to Netty's `ChannelHandlerContext.attr()`, Boost.Asio's associated data, gnet's `Conn` context). Every callback receives the `NetworkConnection *`, from which the caller can retrieve `user_data`.

**Alternatives considered:**
- Single `on_event(connection, event_type, ...)` dispatch function: loses type safety. Rejected.
- Netty-style linked handler chain: over-engineered for a C module. Rejected.
- No user-data pointer (caller uses global/static state): forces awkward patterns, breaks when multiple connections exist. Rejected.

### D5 — `NetworkAddress` parsed at call site, not lazily

**Decision:** `fun_network_address_parse(str, &addr)` converts a `"host:port"` or `"[::1]:port"` string into a `NetworkAddress` struct (family, raw bytes, port) at the call site. No lazy resolution. Numeric IPs only in v1 — hostname strings are rejected with an error.

**Rationale:** Deferred parsing hides errors. Eager parsing keeps the hot path free of string processing.

### D6 — Separate `on_read` (TCP) and `on_datagram_read` (UDP) callbacks

**Decision:** `NetworkHandlers` has two read handlers: `on_read(NetworkConnection *, NetworkBufferVector, int)` for TCP and `on_datagram_read(NetworkConnection *, NetworkBuffer, NetworkAddress)` for UDP. Only the relevant one is invoked based on the connection type.

**Rationale:** TCP read delivers a byte stream scattered across a `NetworkBufferVector`; UDP read delivers a single datagram plus the sender's address. A single signature cannot serve both without wasting a parameter (always-NULL address on TCP) or losing information (no sender on UDP). Two distinct handlers keep the API type-safe and self-documenting.

**Alternatives considered:**
- Single `on_read` with a `NetworkAddress *sender` that is NULL for TCP: works but weakens the contract — caller must check NULL. Rejected.
- Union/tagged handler struct: more complex, no real benefit over two function pointers. Rejected.

### D7 — Write backpressure and internal partial-write completion

**Decision:** `fun_network_tcp_send` and `fun_network_tcp_send_vector` return `ERROR_CODE_NETWORK_WOULD_BLOCK` when the kernel buffer is full. The caller must wait for `on_write_complete` before sending again. When the kernel accepts a partial write, the arch layer internally re-arms the write-ready event and continues until all bytes are sent, then invokes `on_write_complete` once.

**Rationale:** Exposing partial writes to the caller forces every user to implement a retry loop — boilerplate that the arch layer can handle once. Returning would-block (rather than silently queuing) keeps the no-internal-allocation guarantee: the caller's buffer must remain valid until `on_write_complete`, so the caller controls the memory.

**Alternatives considered:**
- Internal send queue (copies data, violates zero-alloc). Rejected.
- Expose partial writes directly (caller resends remainder): error-prone, moves complexity to every call site. Rejected.

### D8 — Re-entrancy from callbacks

**Decision:** Callbacks MAY call `fun_network_tcp_send`, `fun_network_tcp_send_vector`, `fun_network_tcp_close`, `fun_network_udp_close`, and `fun_network_loop_stop`. These calls are deferred: the action is recorded and executed after the current callback returns, within the same dispatch cycle. This avoids recursive dispatch while allowing natural request/response patterns.

**Rationale:** Every real application sends responses from within read handlers and closes connections from error handlers. Forbidding re-entrant calls would force callers into an awkward "queue actions, apply later" pattern that the module can handle internally.

### D9 — Connect timeout via internal timer

**Decision:** `fun_network_tcp_connect` accepts a `timeout_ms` parameter. The arch layer tracks the connect start time and checks for expiry on each loop iteration. On expiry, it cancels the connection and invokes `on_error` with `ERROR_CODE_NETWORK_CONNECT_TIMEOUT`. A value of 0 means no timeout.

**Rationale:** Without a timeout, a connect to a black-hole IP blocks the loop indefinitely. A general-purpose timer API is out of scope for v1; a connect-specific timeout is the minimal solution.

### D10 — `fun_network_loop_run_once` timeout parameter

**Decision:** `fun_network_loop_run_once(loop, timeout_ms)` passes the timeout directly to `epoll_wait` / `GetQueuedCompletionStatus`. 0 = pure poll (return immediately), -1 = block until event, positive = wait up to N ms.

**Rationale:** Without a timeout, callers must either busy-loop (CPU waste) or sleep externally (adds latency). Both `epoll_wait` and `GetQueuedCompletionStatus` accept a timeout natively, so this is zero-cost to expose.

## Risks / Trade-offs

- **IOCP complexity on Windows** → The Windows arch file is more complex than the Linux one (overlapped I/O, per-op OVERLAPPED structs). Mitigation: isolate all IOCP bookkeeping in the arch layer; the core module sees only the common callback contract.
- **Fixed `NETWORK_LOOP_SIZE` / `NETWORK_CONNECTION_SIZE` breaks ABI if internals grow** → Adding a field to the internal struct requires a header bump. Mitigation: pad sizes generously (e.g., 512 bytes for `NetworkLoop`, 256 for `NetworkConnection`) so additions within the pad don't break callers.
- **No server support** → Callers who need inbound connections must wait for a future change. Documented as a non-goal.
- **Single-threaded loop** → High-throughput servers will hit limits. Acceptable for a client-oriented module; multi-loop is a future concern.

## Migration Plan

New module only; no existing code changes. Consumers opt in by including `network/network.h` and linking `ws2_32` (Windows only). Build scripts updated to include arch source files.

## Open Questions

- Should `NetworkLoop` expose a file descriptor (epoll fd / IOCP handle) so callers can integrate it into a larger event loop? Deferred — implement as standalone first.
- ~~`fun_network_address_parse` for hostnames requires getaddrinfo (stdlib). Should we restrict to numeric IPs only for now?~~ **Resolved: numeric IPs only in v1.** Hostname strings are rejected with an error. Hostname resolution is a future capability.
