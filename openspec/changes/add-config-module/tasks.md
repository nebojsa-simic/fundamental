## 1. Header and API Definitions

- [ ] 1.1 Create include/config/config.h with function declarations
- [ ] 1.2 Define Config struct with app_name, ini_path, and internal state fields
- [ ] 1.3 Define DEFINE_RESULT_TYPE for Config, String, int64_t, bool
- [ ] 1.4 Declare fun_config_load(String app_name) API
- [ ] 1.5 Declare fun_config_get_string(Config*, String key) API
- [ ] 1.6 Declare fun_config_get_int(Config*, String key) API
- [ ] 1.7 Declare fun_config_get_bool(Config*, String key) API
- [ ] 1.8 Declare fun_config_get_string_or_default(Config*, String key, String default) API
- [ ] 1.9 Declare fun_config_get_int_or_default(Config*, String key, int64_t default) API
- [ ] 1.10 Declare fun_config_get_bool_or_default(Config*, String key, bool default) API
- [ ] 1.11 Declare fun_config_has(Config*, String key) API
- [ ] 1.12 Declare fun_config_destroy(Config*) API
- [ ] 1.13 Define error codes for config module (ERROR_CODE_CONFIG_KEY_NOT_FOUND, ERROR_CODE_CONFIG_PARSE_ERROR)

## 2. Core Config Module Implementation

- [ ] 2.1 Create src/config/config.c with main config structure
- [ ] 2.2 Implement fun_config_load to initialize Config struct
- [ ] 2.3 Implement internal cascade resolution logic (CLI → env → INI)
- [ ] 2.4 Implement command-line argument parsing for --config:key=value format
- [ ] 2.5 Implement INI file parser using fun_read_file_in_memory()
- [ ] 2.6 Implement INI line parser (handle comments, whitespace, quoted values)
- [ ] 2.7 Implement key-value storage using hashmap internally
- [ ] 2.8 Implement fun_config_get_string with cascade lookup
- [ ] 2.9 Implement fun_config_get_int with string-to-int conversion
- [ ] 2.10 Implement fun_config_get_bool with boolean parsing (true/false/1/0/yes/no)
- [ ] 2.11 Implement fun_config_get_*_or_default wrapper functions
- [ ] 2.12 Implement fun_config_has existence check
- [ ] 2.13 Implement fun_config_destroy to free all internal memory
- [ ] 2.14 Add NULL parameter validation for all public functions

## 3. INI File Parser Implementation

- [ ] 3.1 Create src/config/iniParser.c with INI parsing functions
- [ ] 3.2 Implement fun_ini_parse(String content, HashMap* out_pairs)
- [ ] 3.3 Handle comment lines starting with ; or #
- [ ] 3.4 Handle blank lines and whitespace-only lines
- [ ] 3.5 Parse key=value lines with whitespace trimming
- [ ] 3.6 Handle quoted values (remove surrounding quotes)
- [ ] 3.7 Handle multi-line values if quoted
- [ ] 3.8 Skip malformed lines gracefully (log warning, continue)
- [ ] 3.9 Return error only for critical failures (out of memory)

## 4. Command-Line Argument Parser

- [ ] 4.1 Create src/config/cliParser.c with CLI parsing functions
- [ ] 4.2 Implement fun_cli_parse_args(int argc, const char** argv, HashMap* out_pairs)
- [ ] 4.3 Parse --config:key=value format
- [ ] 4.4 Handle --config:key="value with spaces" quoted values
- [ ] 4.5 Handle multiple --config arguments
- [ ] 4.6 Later arguments override earlier for same key
- [ ] 4.7 Skip non-config arguments (don't error on unknown flags)
- [ ] 4.8 Handle malformed --config arguments gracefully

## 5. Environment Variable Access (Linux)

- [ ] 5.1 Create arch/config/linux-amd64/env.c
- [ ] 5.2 Implement fun_env_get_string(String key) using getenv()
- [ ] 5.3 Implement key transformation: "database.host" → "DATABASE_HOST"
- [ ] 5.4 Handle missing env vars (return NULL or empty result)
- [ ] 5.5 Add NULL pointer checks for safety

## 6. Environment Variable Access (Windows)

- [ ] 6.1 Create arch/config/windows-amd64/env.c
- [ ] 6.2 Implement fun_env_get_string(String key) using GetEnvironmentVariableA()
- [ ] 6.3 Implement key transformation: "database.host" → "DATABASE_HOST"
- [ ] 6.4 Handle buffer size for GetEnvironmentVariableA
- [ ] 6.5 Handle missing env vars (return NULL or empty result)

## 7. App Name and Prefix Handling

- [ ] 7.1 Implement app_name to env prefix conversion
- [ ] 7.2 Convert "myapp" → "MYAPP" (uppercase)
- [ ] 7.3 Build full env var name: prefix + "_" + transformed_key
- [ ] 7.4 Handle app_name validation (reject NULL, empty)
- [ ] 7.5 Document naming convention in header comments

## 8. INI File Location

- [ ] 8.1 Implement executable directory detection
- [ ] 8.2 Build INI path: executable_dir + "/" + app_name + ".ini"
- [ ] 8.3 Use fun_path_join from filesystem module
- [ ] 8.4 Handle platform-specific path separators
- [ ] 8.5 Check file existence before attempting to read

## 9. Boolean Value Parsing

- [ ] 9.1 Implement fun_parse_bool(String value, bool* out_result)
- [ ] 9.2 Accept "true", "1", "yes" as true (case-insensitive)
- [ ] 9.3 Accept "false", "0", "no" as false (case-insensitive)
- [ ] 9.4 Return PARSE_ERROR for invalid boolean strings
- [ ] 9.5 Add unit tests for all boolean variants

## 10. Integer Value Parsing

- [ ] 10.1 Implement fun_parse_int(String value, int64_t* out_result)
- [ ] 10.2 Use fun_string_to_int conversion from string module
- [ ] 10.3 Handle negative integers
- [ ] 10.4 Return PARSE_ERROR for non-numeric strings
- [ ] 10.5 Handle integer overflow detection

## 11. Error Handling and Codes

- [ ] 11.1 Add config-specific error codes to include/error/error.h
- [ ] 11.2 Define ERROR_CODE_CONFIG_KEY_NOT_FOUND
- [ ] 11.3 Define ERROR_CODE_CONFIG_PARSE_ERROR
- [ ] 11.4 Define ERROR_CODE_CONFIG_INVALID_APP_NAME
- [ ] 11.5 Create error result constants with descriptive messages
- [ ] 11.6 Document all error codes in header comments

## 12. Memory Management

- [ ] 12.1 Ensure all allocated memory tracked in Config struct
- [ ] 12.2 Implement proper cleanup in fun_config_destroy
- [ ] 12.3 Free hashmap and all keys/values
- [ ] 12.4 Free INI file content buffer
- [ ] 12.5 Use fun_memory_free for all deallocation
- [ ] 12.6 Add memory safety checks (double-free prevention)

## 13. Tests - Core Config Loading

- [ ] 13.1 Create tests/config/ directory structure
- [ ] 13.2 Create tests/config/build-windows-amd64.bat
- [ ] 13.3 Create tests/config/build-linux-amd64.sh
- [ ] 13.4 Implement test_config_load_success (basic load with no sources)
- [ ] 13.5 Implement test_config_load_null_app_name (error case)
- [ ] 13.6 Implement test_config_load_empty_app_name (error case)
- [ ] 13.7 Implement test_config_destroy_valid (cleanup succeeds)

## 14. Tests - INI File Parsing

- [ ] 13.8 Implement test_config_load_from_ini (INI file exists)
- [ ] 13.9 Implement test_config_ini_missing_file (graceful handling)
- [ ] 13.10 Implement test_config_ini_comments (ignore comment lines)
- [ ] 13.11 Implement test_config_ini_whitespace (trim spaces around =)
- [ ] 13.12 Implement test_config_ini_quoted_values (preserve quoted content)
- [ ] 13.13 Implement test_config_ini_malformed_lines (skip gracefully)

## 15. Tests - Environment Variables

- [ ] 15.1 Implement test_config_get_from_env (env var set)
- [ ] 15.2 Implement test_config_env_key_transformation (dots to underscores)
- [ ] 15.3 Implement test_config_env_missing (cascade to next source)
- [ ] 15.4 Implement test_config_env_prefix_correct (MYAPP_ prefix)

## 16. Tests - Command-Line Arguments

- [ ] 16.1 Implement test_config_get_from_cli (--config:key=value)
- [ ] 16.2 Implement test_config_cli_multiple_args (multiple --config flags)
- [ ] 16.3 Implement test_config_cli_override_env (CLI wins over env)
- [ ] 16.4 Implement test_config_cli_quoted_values (spaces in values)

## 17. Tests - Cascade Priority

- [ ] 17.1 Implement test_config_cli_overrides_all (CLI > env > INI)
- [ ] 17.2 Implement test_config_env_overrides_ini (env > INI)
- [ ] 17.3 Implement test_config_ini_fallback (INI used when others missing)
- [ ] 17.4 Implement test_config_all_sources_missing (KEY_NOT_FOUND error)

## 18. Tests - Type-Safe Getters

- [ ] 18.1 Implement test_config_get_string_success
- [ ] 18.2 Implement test_config_get_string_missing
- [ ] 18.3 Implement test_config_get_int_success
- [ ] 18.4 Implement test_config_get_int_invalid (parse error)
- [ ] 18.5 Implement test_config_get_bool_true (all true variants)
- [ ] 18.6 Implement test_config_get_bool_false (all false variants)
- [ ] 18.7 Implement test_config_get_bool_invalid (parse error)

## 19. Tests - Get With Default

- [ ] 19.1 Implement test_config_get_string_or_default_uses_default
- [ ] 19.2 Implement test_config_get_string_or_default_uses_value (ignores default)
- [ ] 19.3 Implement test_config_get_int_or_default_uses_default
- [ ] 19.4 Implement test_config_get_bool_or_default_uses_default

## 20. Tests - Existence Check

- [ ] 20.1 Implement test_config_has_returns_true (key exists)
- [ ] 20.2 Implement test_config_has_returns_false (key missing)
- [ ] 20.3 Implement test_config_has_does_not_return_value (type-agnostic)

## 21. Documentation

- [ ] 21.1 Add function documentation comments in config.h
- [ ] 21.2 Document cascade priority in header
- [ ] 21.3 Document error codes and their meanings
- [ ] 21.4 Document INI file format in comments
- [ ] 21.5 Document CLI argument format (--config:key=value)
- [ ] 21.6 Document environment variable naming convention
- [ ] 21.7 Add usage examples in header comments
- [ ] 21.8 Update README.md with config module overview
- [ ] 21.9 Document platform-specific considerations (env var access)
