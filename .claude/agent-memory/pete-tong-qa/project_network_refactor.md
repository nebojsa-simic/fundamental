---
name: Network module refactor QA status
description: QA findings for refactor-network-simple-async — new simple async TCP/UDP API replacing reactor, verified 2026-03-15
type: project
---

Network module was rewritten to a simple async TCP/UDP interface (replacing the reactor/event-loop API) and verified clean on 2026-03-15.

**Change:** refactor-network-simple-async — replaced NetworkLoop/NetworkHandlers reactor with pool-based TcpNetworkConnection and self-polling AsyncResult poll functions. Old reactor API fully removed from all headers, src, and arch layers.

**Test command (network):**
```bash
GCC="C:/Users/nsimi/AppData/Local/Microsoft/WinGet/Packages/BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe/mingw64/bin/gcc.exe"
ROOT="/c/Users/nsimi/workspaces-win/fundamental"
"$GCC" --std=c17 -O0 -g -I"$ROOT/include" -Wall -Wextra \
  "$ROOT/tests/network/test_network.c" \
  "$ROOT/src/network/network.c" \
  "$ROOT/arch/network/windows-amd64/network.c" \
  "$ROOT/src/async/async.c" \
  "$ROOT/arch/memory/windows-amd64/memory.c" \
  "$ROOT/src/string/stringOperations.c" \
  "$ROOT/src/string/stringValidation.c" \
  -o "$ROOT/tests/network/test.exe" -lws2_32 -lmswsock && "$ROOT/tests/network/test.exe"
```

**Results:** 5/5 network tests pass. 7/7 async tests pass. 7/7 process tests pass.

**Tests:** test_address_parse, test_address_format, test_connect_fails, test_tcp_round_trip, test_udp_send.

**Pre-existing warnings (not caused by network refactor):**
- stringOperations.c: unused variable `original_begin` — pre-existing, unrelated

**Architecture notes:**
- Connection pool: 16 slots, static allocation, `pool_acquire` / `pool_release`
- rx_buf: 4096-byte staging buffer allocated on successful connect, freed on close
- `fun_network_tcp_close(NULL)` returns ERROR_RESULT_NULL_POINTER safely — NULL check present
- `poll_recv_exact` drains rx_buf before reading from socket — correct order confirmed
- `response->length` set to `bytes` (not received count) on ASYNC_COMPLETED — correct
- Error paths in `pool_connect`: rx_buf alloc failure releases pool slot before returning error
- Windows arch: uses `select()` for poll_connect instead of WSAPoll() (known unreliable on some Windows versions — intentional)

**Why:** Network module refactored to eliminate reactor complexity; simple async is more composable with the existing AsyncResult pattern.
**How to apply:** When testing network changes, use the build command above. Watch the rx_buf overflow/compaction path in poll_recv_exact — it's the most complex logic in the module.
