@ECHO OFF
REM Minimal Console Demo - Build Script
REM Dependencies: console, string (operations), memory
gcc --std=c17 -Os -I ../../include demo.c ^
    ../../src/console/console.c ^
    ../../src/string/stringOperations.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -o demo.exe
strip --strip-unneeded demo.exe
