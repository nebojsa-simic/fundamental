@ECHO OFF
REM Minimal Console Demo - Build Script
REM Dependencies: console, string (operations), memory
gcc --std=c17 -Os -nostdlib -ffreestanding -mno-stack-arg-probe ^
    -Wl,--subsystem,console ^
    -I ../../include demo.c ^
    ../../arch/startup/windows-amd64/windows.c ^
    ../../src/console/console.c ^
    ../../src/string/stringOperations.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -lkernel32 ^
    -o demo.exe
strip --strip-unneeded demo.exe
