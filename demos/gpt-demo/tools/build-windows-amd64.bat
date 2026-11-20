@echo off
gcc --std=c17 -Os -nostdlib -ffreestanding -mno-stack-arg-probe ^
    -Wl,--subsystem,console ^
    -I ../../../include inspect_gguf.c ^
    ../../../arch/startup/windows-amd64/windows.c ^
    ../../../arch/file/windows-amd64/fileRead.c ^
    ../../../arch/file/windows-amd64/fileReadMmap.c ^
    ../../../arch/file/windows-amd64/fileReadRing.c ^
    ../../../src/async/async.c ^
    ../../../arch/async/windows-amd64/async.c ^
    ../../../src/console/console.c ^
    ../../../arch/console/windows-amd64/console.c ^
    ../../../arch/memory/windows-amd64/memory.c ^
    ../../../src/string/stringConversion.c ^
    ../../../src/string/stringOperations.c ^
    -lkernel32 ^
    -o inspect_gguf.exe
if %ERRORLEVEL% EQU 0 echo Build complete: inspect_gguf.exe
