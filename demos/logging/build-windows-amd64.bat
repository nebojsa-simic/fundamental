@ECHO OFF
REM Minimal Logging Demo - Build Script
REM Dependencies: logging, console, string (all 4 files), memory
gcc --std=c17 -Os -nostdlib -ffreestanding -mno-stack-arg-probe ^
    -Wl,--subsystem,console ^
    -I ../../include ^
    -D FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_DEBUG ^
    -D FUNDAMENTAL_LOG_OUTPUT_CONSOLE=1 ^
    -D FUNDAMENTAL_LOG_OUTPUT_FILE=0 ^
    demo.c ^
    ../../arch/startup/windows-amd64/windows.c ^
    ../../src/logging/logging.c ^
    ../../src/console/console.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/string/stringValidation.c ^
    ../../arch/logging/windows-amd64/logging.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -lkernel32 ^
    -o demo.exe
strip --strip-unneeded demo.exe
