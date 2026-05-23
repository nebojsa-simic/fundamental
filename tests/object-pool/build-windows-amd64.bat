@echo off
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../src/object-pool/object-pool.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -o test.exe

if %ERRORLEVEL% EQU 0 echo Build complete: test.exe
