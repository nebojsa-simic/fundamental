@ECHO OFF
REM Shutdown Framework Tests - Build Script
REM Dependencies: shutdown, console, memory, atomic, string

gcc --std=c17 -Os -I ../../include test.c ^
    ../../src/shutdown/shutdown.c ^
    ../../src/console/console.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../arch/shutdown/windows-amd64/atomic.c ^
    ../../arch/signals/windows-amd64/signal.c ^
    -lkernel32 ^
    -o test.exe

strip --strip-unneeded test.exe
