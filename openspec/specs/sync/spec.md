# sync Specification

## Purpose
TBD - created by archiving change sync. Update Purpose after archive.
## Requirements
### Requirement: Mutex can be created and destroyed
The system SHALL provide `fun_mutex_create(&out_mutex)` that allocates and initializes a `Mutex`. On success, `*out_mutex` is set to a valid handle. On error, `*out_mutex` is unchanged. The system SHALL provide `fun_mutex_destroy(mutex)` that frees the mutex. Destroy on NULL SHALL be a no-op. Destroying a mutex while locked SHALL be undefined.

#### Scenario: Mutex created successfully
- **WHEN** `fun_mutex_create` is called with a valid output pointer
- **THEN** the result SHALL be OK and `*out_mutex` SHALL be a valid handle

#### Scenario: NULL output pointer returns error
- **WHEN** `fun_mutex_create` is called with a NULL output pointer
- **THEN** the function SHALL return an error result

#### Scenario: Destroy NULL mutex is a no-op
- **WHEN** `fun_mutex_destroy` is called with NULL
- **THEN** the function SHALL return without error

### Requirement: Mutex can be locked and unlocked
The system SHALL provide `fun_mutex_lock(mutex)` that acquires the mutex, blocking if already held by another thread. The system SHALL provide `fun_mutex_unlock(mutex)` that releases the mutex.

#### Scenario: Lock blocks until mutex is available
- **WHEN** thread A holds the mutex and thread B calls `fun_mutex_lock` on the same mutex
- **THEN** thread B SHALL block until thread A calls `fun_mutex_unlock`

#### Scenario: Lock succeeds when mutex is free
- **WHEN** `fun_mutex_lock` is called on an unlocked mutex
- **THEN** the call SHALL return immediately with the mutex held

### Requirement: Condition variable can be created and destroyed
The system SHALL provide `fun_condvar_create(&out_cv)` that allocates and initializes a `CondVar`. On success, `*out_cv` is set to a valid handle. The system SHALL provide `fun_condvar_destroy(cv)` that frees the condition variable. Destroy on NULL SHALL be a no-op.

#### Scenario: CondVar created successfully
- **WHEN** `fun_condvar_create` is called with a valid output pointer
- **THEN** the result SHALL be OK and `*out_cv` SHALL be a valid handle

### Requirement: Thread can wait on a condition variable
The system SHALL provide `fun_condvar_wait(cv, mutex)` that atomically releases the mutex and blocks the calling thread until signaled. Upon wake, the mutex SHALL be re-acquired before the function returns. The mutex MUST be held by the calling thread when `fun_condvar_wait` is called.

#### Scenario: Wait blocks until signaled
- **WHEN** thread A calls `fun_condvar_wait` on a condvar with an associated mutex held
- **THEN** thread A SHALL atomically release the mutex and block

#### Scenario: Signal wakes a waiting thread
- **WHEN** thread A is blocked in `fun_condvar_wait` and thread B calls `fun_condvar_signal` on the same condvar
- **THEN** thread A SHALL wake, re-acquire the mutex, and return from `fun_condvar_wait`

### Requirement: Condition variable can be signaled and broadcast
The system SHALL provide `fun_condvar_signal(cv)` that wakes at least one thread waiting on the condvar. The system SHALL provide `fun_condvar_broadcast(cv)` that wakes all threads waiting on the condvar.

#### Scenario: Signal wakes one waiting thread
- **WHEN** two threads are blocked in `fun_condvar_wait` on the same condvar and `fun_condvar_signal` is called
- **THEN** at least one of the waiting threads SHALL wake

#### Scenario: Broadcast wakes all waiting threads
- **WHEN** two threads are blocked in `fun_condvar_wait` on the same condvar and `fun_condvar_broadcast` is called
- **THEN** all waiting threads SHALL wake

