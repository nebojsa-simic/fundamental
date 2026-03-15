## Context

The network module currently implements a full reactor model: `NetworkLoop` (epoll/IOCP), `NetworkHandlers` (callback table), and `NetworkConnection` (opaque, caller-allocated). This is appropriate for servers managing thousands of concurrent connections. The primary consumers of this library are CLI tools that make a small, fixed number of outbound TCP/UDP requests in a straightforward request/response pattern. The callback inversion-of-control model forces that logic to be split across `on_connect`, `on_read`, and `on_write_complete` functions, which is harder to write and reason about than a linear async flow.

## Goals / Non-Goals

**Goals:**
- Replace the entire public network API with a simple async interface: connect, send, receive-exact, receive-until, close
- Compose naturally with the existing `AsyncResult` / `fun_async_await` infrastructure
- Support the two-phase framing pattern required by MySQL, HTTP, gRPC, and similar protocols
- Handle split delimiters, leftover bytes, and unbounded-growth protection transparently
- Allow staging buffer tuning via `fun.ini` without an API change

**Non-Goals:**
- Server-side accept / listen
- High-concurrency scenarios (thousands of simultaneous connections)
- Scatter/gather vectored I/O (removed; single-buffer API is sufficient for CLI use)
- Async DNS resolution (numeric IP only, unchanged)
- UDP receive (fire-and-forget send only for now)

## Decisions

### Decision 1: Self-polling AsyncResult instead of a shared external reactor

**Chosen:** Each `AsyncResult` returned by a network function carries a `poll` function that calls `WSAPoll` / `poll` directly on its socket with `timeout_ms = 0`. `fun_async_await` spins calling `result->poll(result)`. `fun_async_await_all` selects across all sockets simultaneously, preserving multi-connection parallelism.

**Alternative considered:** Keep an internal per-process or per-thread reactor (epoll/IOCP) and have `poll` feed through it.

**Rationale:** For CLI workloads with a handful of connections, per-socket `WSAPoll`/`poll` is negligible cost. Removing the reactor eliminates all shared state, lifecycle management, and threading concerns. The existing `AsyncResult` contract is preserved with no new abstractions. If a future use case needs thousands of connections, a server-oriented module can be added separately.

### Decision 2: `TcpNetworkConnection` as an opaque pointer typedef

**Chosen:** `typedef struct TcpNetworkConnection_s *TcpNetworkConnection`. The internal reactor allocates connection objects from a per-thread pool at connect time and returns the handle via `OutputTcpNetworkConnection`. The caller passes `TcpNetworkConnection` by value to send/receive (pointer-sized, cheap).

**Alternative considered:** Caller-allocated opaque struct with `NETWORK_TCP_CONNECTION_SIZE` constant (prior design).

**Rationale:** Caller-allocated structs require the caller to manage storage lifetimes explicitly. Since the reactor owns the staging buffer (see Decision 3), the connection already has internal state the caller cannot meaningfully pre-allocate. Pointer handle is consistent with the `Memory = void*` convention in the memory module and avoids exposing `NETWORK_TCP_CONNECTION_SIZE`.

### Decision 3: Connection-owned overflow buffer for receive operations

**Chosen:** Each `TcpNetworkConnection` owns a small internal `rx_buf`. When `recv()` returns more bytes than `receive_exact` requested, the surplus is stored in `rx_buf` and drained first on the next `receive_exact` call.

**Alternative considered:** Require callers to pass a large enough buffer that `recv()` never overflows it.

**Rationale:** `recv()` returns whatever the OS has available — there is no way for the caller to guarantee an exact read without buffering. The overflow buffer keeps this concern out of caller code. Unlike the previous design, this buffer does not need to support delimiter scanning; it only needs to hold surplus bytes between calls, so the implementation is trivial.

### Decision 4: Staging buffer default and tuning via `fun.ini`

**Chosen:** The internal reactor initialises the overflow buffer at a compile-time default (4 KB). Power users override the initial allocation via `[network] rx_buf_size` in `fun.ini`, loaded through `fun_config_load()` at reactor init time.

**Alternative considered:** Expose an `rx_buf_size` parameter in `fun_network_tcp_connect`.

**Rationale:** Adding a size parameter to `connect` couples an implementation detail to the primary API and adds noise to the common case. Config-module integration keeps the API surface minimal and is consistent with how other modules will expose tunables.

## Risks / Trade-offs

- **Poll efficiency vs. epoll** → For CLI workloads (< 10 connections), WSAPoll/poll cost is unmeasurable. Acceptable.
- **Per-thread reactor pool size** → Fixed-size pool means a hard cap on simultaneous connections per thread. Default of 16 slots is sufficient for CLI use; configurable via `fun.ini`. Exceeding the pool returns `ERROR_CODE_NETWORK_SEND_FAILED` from `connect`.
- **No backpressure on send** → The old API had `ERROR_CODE_NETWORK_WOULD_BLOCK`. The new `fun_network_tcp_send` blocks internally (via the poll loop) until the kernel buffer accepts all bytes. This simplifies the API at the cost of no explicit flow control signal. Acceptable for CLI use; a future server module would need to reintroduce it.

## Migration Plan

No other modules or callers in this repository depend on the network module. The change is a full in-place replacement. Steps:

1. Rewrite `include/network/network.h` (new API, remove all old types and functions)
2. Rewrite `src/network/network.c` (new core logic, staging buffer, AsyncResult poll fns)
3. Rewrite `arch/network/windows-amd64/network.c` (WSAPoll-based self-polling)
4. Rewrite `arch/network/linux-amd64/network.c` (poll-based self-polling)
5. Rewrite `tests/network/` (new test suite covering all new requirements)
6. Update `openspec/specs/network/spec.md` and add `openspec/specs/network-simple-async/spec.md`
7. Update `.opencode/skills/fundamental-network.md`

Rollback: git revert. No other modules are affected.
