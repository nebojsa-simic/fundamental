## ADDED Requirements

### Requirement: Notification State Flags

The file notification state SHALL track each resource with explicit flags.

#### Scenario: FileNotificationState flags
- **WHEN** `FileNotificationState` is defined
- **THEN** it contains `inotify_opened` and `watch_registered` boolean flags

### Requirement: Unregister Cleanup

The unregister function SHALL properly clean up all allocated resources.

#### Scenario: Watch registered
- **WHEN** `fun_unregister_file_change_notification()` is called with `watch_registered == true`
- **THEN** `inotify_rm_watch()` is called with the watch fd

#### Scenario: inotify fd opened
- **WHEN** `fun_unregister_file_change_notification()` is called with `inotify_opened == true`
- **THEN** `close()` is called on the inotify fd

#### Scenario: State memory
- **WHEN** `fun_unregister_file_change_notification()` completes
- **THEN** the `FileNotificationState` memory is freed

### Requirement: Flag Clearing

Flags SHALL be cleared after each resource is freed.

#### Scenario: After watch removal
- **WHEN** `inotify_rm_watch()` is called
- **THEN** `watch_registered` is set to `false`

#### Scenario: After fd close
- **WHEN** `close()` is called on inotify fd
- **THEN** `inotify_opened` is set to `false`

### Requirement: NULL Handling

The unregister function SHALL handle NULL inputs gracefully.

#### Scenario: NULL file path
- **WHEN** `fun_unregister_file_change_notification(NULL, state)` is called
- **THEN** `ERROR_RESULT_NULL_POINTER` is returned

#### Scenario: NULL state
- **WHEN** `fun_unregister_file_change_notification(filePath, NULL)` is called
- **THEN** `ERROR_RESULT_NULL_POINTER` is returned

### Requirement: Multiple Unregister Calls

Multiple calls to unregister SHALL not cause crashes or errors.

#### Scenario: Double unregister
- **WHEN** `fun_unregister_file_change_notification()` is called twice
- **THEN** second call returns success (idempotent behavior)

### Requirement: Unregister Without Register

Calling unregister without prior register SHALL be handled safely.

#### Scenario: No prior registration
- **WHEN** `fun_unregister_file_change_notification()` is called on fresh state
- **THEN** function returns success (no resources to clean up)
