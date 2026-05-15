## 1. Header and Module Structure

- [x] 1.1 Create `include/fundamental/sync/sync.h` with `Mutex` and `CondVar` opaque types and all function declarations
- [x] 1.2 Create `src/sync/` directory
- [x] 1.3 Create `arch/sync/windows-amd64/` directory with `sync.c` for Windows platform
- [x] 1.4 Create `arch/sync/linux-amd64/` directory with `sync.c` for Linux platform

## 2. Mutex Implementation (Windows)

- [x] 2.1 Implement `fun_mutex_create()` — allocate and initialize `CRITICAL_SECTION`
- [x] 2.2 Implement `fun_mutex_lock()` — `EnterCriticalSection`
- [x] 2.3 Implement `fun_mutex_unlock()` — `LeaveCriticalSection`
- [x] 2.4 Implement `fun_mutex_destroy()` — `DeleteCriticalSection`, free handle

## 3. Mutex Implementation (Linux)

- [x] 3.1 Implement `fun_mutex_create()` — allocate and set lock = 0
- [x] 3.2 Implement `fun_mutex_lock()` — atomic CAS + futex(`FUTEX_WAIT`)
- [x] 3.3 Implement `fun_mutex_unlock()` — atomic store + futex(`FUTEX_WAKE`)
- [x] 3.4 Implement `fun_mutex_destroy()` — free handle

## 4. CondVar Implementation (Windows)

- [x] 4.1 Implement `fun_condvar_create()` — allocate and initialize `CONDITION_VARIABLE`
- [x] 4.2 Implement `fun_condvar_wait()` — `SleepConditionVariableCS`
- [x] 4.3 Implement `fun_condvar_signal()` — `WakeConditionVariable`
- [x] 4.4 Implement `fun_condvar_broadcast()` — `WakeAllConditionVariable`
- [x] 4.5 Implement `fun_condvar_destroy()` — no OS destroy needed for CONDITION_VARIABLE, free handle

## 5. CondVar Implementation (Linux)

- [x] 5.1 Implement `fun_condvar_create()` — allocate and set seq = 0
- [x] 5.2 Implement `fun_condvar_wait()` — capture seq, unlock mutex, futex(`FUTEX_WAIT`) on seq, re-lock mutex
- [x] 5.3 Implement `fun_condvar_signal()` — atomic inc seq + futex(`FUTEX_WAKE`, 1)
- [x] 5.4 Implement `fun_condvar_broadcast()` — atomic inc seq + futex(`FUTEX_WAKE`, INT_MAX)
- [x] 5.5 Implement `fun_condvar_destroy()` — free handle

## 6. Build Scripts

- [x] 6.1 Create `tests/sync/build-windows-amd64.bat`
- [x] 6.2 Create `tests/sync/build-linux-amd64.sh` (test binary links `-lpthread` for test threads only; library code uses futex)

## 7. Tests

- [x] 7.1 Test mutex create and destroy
- [x] 7.2 Test mutex create with NULL output returns error
- [x] 7.3 Test mutex destroy NULL is no-op
- [x] 7.4 Test mutex lock/unlock sequence
- [x] 7.5 Test mutex lock blocks another thread
- [x] 7.6 Test condvar create and destroy
- [x] 7.7 Test condvar wait/signal — waiting thread wakes
- [x] 7.8 Test condvar broadcast — all waiting threads wake

## 8. Validation

- [x] 8.1 Run `run-tests-windows-amd64.bat` — all sync tests pass
- [ ] 8.2 Run `./run-tests-linux-amd64.sh` — all sync tests pass (requires Linux)
- [x] 8.3 Run `code-format.bat` — clang-format passes on all new files
- [x] 8.4 Run `openspec validate sync` — all specs validated
