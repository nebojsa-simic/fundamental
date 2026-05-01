@ECHO OFF

REM Compile logging module tests
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    -D FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_TRACE ^
    -D FUNDAMENTAL_LOG_OUTPUT_CONSOLE=1 ^
    -D FUNDAMENTAL_LOG_OUTPUT_FILE=1 ^
    -D FUNDAMENTAL_LOG_BUFFER_SIZE=512 ^
    test.c ^
    ../../src/logging/logging.c ^
    ../../arch/logging/windows-amd64/logging.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/string/stringValidation.c ^
    ../../src/console/console.c ^
    ../../src/config/config.c ^
    ../../src/file/file.c ^
    ../../src/error/error.c ^
    ../../src/startup/startup.c ^
    ../../arch/string/windows-amd64/string.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/config/windows-amd64/env.c ^
    ../../arch/file/windows-amd64/fileRead.c ^
    ../../arch/file/windows-amd64/fileWrite.c ^
    ../../arch/file/windows-amd64/fileAppend.c ^
    ../../arch/error/windows-amd64/error.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../arch/startup/windows-amd64/windows.c ^
    ../../arch/platform/windows-amd64/platform.c ^
    ../../arch/filesystem/windows-amd64/path.c ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe
