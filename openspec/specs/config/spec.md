# Config Module Specification

## Purpose
Config module provides unified configuration management with cascading sources (command-line, environment, INI file) and type-safe accessors for CLI applications built without stdlib dependencies.

## Requirements

### Requirement: Config Load with App Name
The config module SHALL provide function to load configuration from all sources using application name for environment variable prefix.

#### Scenario: Load config successfully
- **WHEN** fun_config_load(app_name) is called with valid app name
- **THEN** Config handle is returned with SUCCESS status
- **AND** configuration sources are cascaded: CLI args → environment → INI file
- **AND** app_name is used to construct environment variable prefix (e.g., MYAPP_ for "myapp")

#### Scenario: Load with NULL app name
- **WHEN** fun_config_load is called with NULL app_name
- **THEN** function returns error with ERROR_CODE_NULL_POINTER

#### Scenario: Load with empty app name
- **WHEN** fun_config_load is called with empty string app_name
- **THEN** function returns error indicating invalid parameter

### Requirement: Command-Line Argument Parsing
The config module SHALL parse command-line arguments with --config: prefix for configuration values.

#### Scenario: Parse --config:key=value argument
- **WHEN** application is invoked with --config:database.host=localhost
- **THEN** key "database.host" has value "localhost" from CLI source
- **AND** CLI value takes highest priority in cascade

#### Scenario: Parse multiple --config arguments
- **WHEN** application has --config:key1=value1 --config:key2=value2
- **THEN** both keys are available in configuration
- **AND** later arguments override earlier arguments for same key

#### Scenario: Handle --config with special characters in value
- **WHEN** argument is --config:connection.string="host=db;port=5432"
- **THEN** value includes all characters after equals sign
- **AND** quoted values preserve internal spaces and semicolons

### Requirement: Environment Variable Access
The config module SHALL read environment variables with APPNAME_ prefix using platform-specific implementations.

#### Scenario: Read environment variable
- **WHEN** MYAPP_DATABASE_HOST environment variable is set to "production.db"
- **AND** fun_config_get_string is called with key "database.host"
- **THEN** value "production.db" is returned from environment source
- **AND** key is transformed: uppercase and dots become underscores

#### Scenario: Environment variable key transformation
- **WHEN** config key is "database.host" and app_name is "myapp"
- **THEN** environment variable searched is MYAPP_DATABASE_HOST
- **AND** dots in key become underscores in env var name

#### Scenario: Missing environment variable
- **WHEN** MYAPP_DATABASE_HOST is not set
- **AND** fun_config_get_string is called with key "database.host"
- **THEN** function continues cascade to next source (INI file)

### Requirement: INI File Parsing
The config module SHALL parse INI files with flat key=value format from executable directory.

#### Scenario: Parse INI file successfully
- **WHEN** myapp.ini exists in executable directory with content "database.host=localhost"
- **AND** fun_config_load("myapp") is called
- **THEN** key "database.host" has value "localhost" from INI source
- **AND** INI file uses lowest priority in cascade

#### Scenario: INI file not found
- **WHEN** myapp.ini does not exist
- **AND** fun_config_load("myapp") is called
- **THEN** load succeeds with empty INI source
- **AND** configuration can still come from CLI or environment

#### Scenario: Parse INI with comments
- **WHEN** INI file contains "; this is a comment" lines
- **THEN** comment lines are ignored during parsing

#### Scenario: Parse INI with whitespace
- **WHEN** INI file contains "database.host = localhost " with spaces around equals
- **THEN** key is "database.host" and value is "localhost" (trimmed)

#### Scenario: Parse INI with quoted values
- **WHEN** INI file contains 'app.name="My Application"'
- **THEN** value is "My Application" without quotes

### Requirement: Cascade Priority
The config module SHALL resolve configuration values with strict priority: CLI > Environment > INI.

#### Scenario: CLI overrides environment and INI
- **WHEN** --config:database.host=cli_value is provided
- **AND** MYAPP_DATABASE_HOST=env_value is set
- **AND** INI has database.host=ini_value
- **THEN** fun_config_get_string returns "cli_value"

#### Scenario: Environment overrides INI
- **WHEN** MYAPP_DATABASE_HOST=env_value is set
- **AND** INI has database.host=ini_value
- **AND** no CLI argument provided
- **THEN** fun_config_get_string returns "env_value"

#### Scenario: INI used when CLI and env not set
- **WHEN** no --config:database.host argument
- **AND** MYAPP_DATABASE_HOST not set
- **AND** INI has database.host=ini_value
- **THEN** fun_config_get_string returns "ini_value"

### Requirement: Get String Value
The config module SHALL provide function to retrieve string configuration values.

#### Scenario: Get existing string value
- **WHEN** fun_config_get_string(config, "database.host") is called
- **AND** key exists in any source
- **THEN** StringResult.value contains the string value
- **AND** StringResult.error indicates SUCCESS

#### Scenario: Get missing string value
- **WHEN** fun_config_get_string(config, "nonexistent.key") is called
- **AND** key does not exist in any source
- **THEN** StringResult.error indicates KEY_NOT_FOUND

### Requirement: Get Integer Value
The config module SHALL provide function to retrieve int64 configuration values.

#### Scenario: Get existing integer value
- **WHEN** fun_config_get_int(config, "database.port") is called
- **AND** key exists with value "5432"
- **THEN** IntResult.value contains 5432
- **AND** IntResult.error indicates SUCCESS

#### Scenario: Get invalid integer value
- **WHEN** fun_config_get_int(config, "database.port") is called
- **AND** key exists with non-numeric value "not_a_number"
- **THEN** IntResult.error indicates PARSE_ERROR

### Requirement: Get Boolean Value
The config module SHALL provide function to retrieve boolean configuration values.

#### Scenario: Get true boolean value
- **WHEN** fun_config_get_bool(config, "debug.enabled") is called
- **AND** key exists with value "true" or "1" or "yes"
- **THEN** BoolResult.value contains true
- **AND** BoolResult.error indicates SUCCESS

#### Scenario: Get false boolean value
- **WHEN** fun_config_get_bool(config, "debug.enabled") is called
- **AND** key exists with value "false" or "0" or "no"
- **THEN** BoolResult.value contains false
- **AND** BoolResult.error indicates SUCCESS

#### Scenario: Get invalid boolean value
- **WHEN** fun_config_get_bool(config, "debug.enabled") is called
- **AND** key exists with value "maybe"
- **THEN** BoolResult.error indicates PARSE_ERROR

### Requirement: Get With Default Value
The config module SHALL provide ergonomic functions to retrieve values with fallback defaults.

#### Scenario: Get string with default
- **WHEN** fun_config_get_string_or_default(config, "app.theme", "default") is called
- **AND** key does not exist
- **THEN** function returns "default"
- **AND** no error is returned

#### Scenario: Get int with default
- **WHEN** fun_config_get_int_or_default(config, "server.port", 8080) is called
- **AND** key does not exist
- **THEN** function returns 8080
- **AND** no error is returned

#### Scenario: Get bool with default
- **WHEN** fun_config_get_bool_or_default(config, "debug.enabled", false) is called
- **AND** key does not exist
- **THEN** function returns false
- **AND** no error is returned

#### Scenario: Get existing value ignores default
- **WHEN** fun_config_get_int_or_default(config, "server.port", 8080) is called
- **AND** key exists with value 5432
- **THEN** function returns 5432 (default is ignored)

### Requirement: Check Key Existence
The config module SHALL provide function to check if a key exists in any source without retrieving value.

#### Scenario: Check existing key
- **WHEN** fun_config_has(config, "database.host") is called
- **AND** key exists in any source
- **THEN** function returns true
- **AND** no error is returned

#### Scenario: Check missing key
- **WHEN** fun_config_has(config, "nonexistent.key") is called
- **AND** key does not exist in any source
- **THEN** function returns false
- **AND** no error is returned

### Requirement: Config Destroy
The config module SHALL provide function to cleanup and free configuration resources.

#### Scenario: Destroy valid config
- **WHEN** fun_config_destroy(config) is called with valid config
- **THEN** all internal memory is freed
- **AND** INI file contents are released
- **AND** function returns SUCCESS

#### Scenario: Destroy NULL config
- **WHEN** fun_config_destroy is called with NULL config
- **THEN** function returns error or no-op gracefully

### Requirement: INI File Location
The config module SHALL locate INI file in the same directory as the executable.

#### Scenario: Find INI in executable directory
- **WHEN** executable is at /usr/bin/myapp
- **AND** INI file is at /usr/bin/myapp.ini
- **THEN** INI file is found and loaded

#### Scenario: Handle missing INI gracefully
- **WHEN** executable is at /usr/bin/myapp
- **AND** myapp.ini does not exist
- **THEN** load succeeds with empty INI source
- **AND** no error is returned

### Requirement: Type-Safe Access
The config module SHALL provide compile-time type safety through separate getter functions.

#### Scenario: Type-safe string access
- **WHEN** fun_config_get_string is called
- **THEN** return type is StringResult (not int or bool)
- **AND** type mismatch is caught at compile time

#### Scenario: Type-safe int access
- **WHEN** fun_config_get_int is called
- **THEN** return type is IntResult (not String or bool)
- **AND** type mismatch is caught at compile time

#### Scenario: Type-safe bool access
- **WHEN** fun_config_get_bool is called
- **THEN** return type is BoolResult (not String or int)
- **AND** type mismatch is caught at compile time

### Requirement: Cross-Platform Consistency
The config module SHALL behave identically across all supported platforms.

#### Scenario: Same cascade behavior on Windows and Linux
- **WHEN** same configuration is provided via CLI, env, and INI
- **AND** application runs on Windows
- **AND** application runs on Linux
- **THEN** cascade priority produces identical results on both platforms

#### Scenario: Platform-specific environment access
- **WHEN** environment variable is accessed
- **THEN** Windows uses GetEnvironmentVariable
- **AND** Linux uses getenv
- **AND** behavior is identical from caller perspective

## Constraints

- Load operation always succeeds unless app_name is NULL or memory allocation fails
- INI file parsing errors skip malformed lines rather than failing entire load
- Keys use dotted notation for hierarchy (database.host, not [database] host)
- Environment variable names are uppercase with underscores (MYAPP_DATABASE_HOST)
- CLI arguments use --config:key=value format
- Boolean parsing accepts: true/false, 1/0, yes/no (case-insensitive)
- Integer parsing accepts decimal integers (no hex or octal in v1)
- All memory allocated by config module must be freed by fun_config_destroy
