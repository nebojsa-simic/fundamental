@ECHO OFF

REM Compile logging module tests
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    -D FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_TRACE ^
    -D FUNDAMENTAL_LOG_OUTPUT_CONSOLE=1 ^
    -D FUNDAMENTAL_LOG_OUTPUT_FILE=1 ^
    -D FUNDAMENTAL_LOG_BUFFER_SIZE=512 ^
    -D FUNDAMENTAL_STARTUP_VERBOSE=1 ^
    test.c ^
    ../../src/logging/logging.c ^
    ../../arch/logging/windows-amd64/logging.c ^
    ../../src/startup/startup.c ^
    ../../src/shutdown/shutdown.c ^
    ../../arch/signals/windows-amd64/signal.c ^
    ../../src/platform/platform.c ^
    ../../arch/platform/windows-amd64/platform.c ^
    ../../src/filesystem/path.c ^
    ../../src/filesystem/file_exists.c ^
    ../../src/filesystem/file_size.c ^
    ../../src/filesystem/directory.c ^
    ../../arch/filesystem/windows-amd64/path.c ^
    ../../arch/filesystem/windows-amd64/file_exists.c ^
    ../../arch/filesystem/windows-amd64/file_size.c ^
    ../../arch/filesystem/windows-amd64/directory.c ^
    ../../src/config/config.c ^
    ../../src/config/iniParser.c ^
    ../../src/config/cliParser.c ^
    ../../arch/config/windows-amd64/env.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/string/stringValidation.c ^
    ../../src/hashmap/hashmap.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -lws2_32 ^
    -lkernel32 ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe
