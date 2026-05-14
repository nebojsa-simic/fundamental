@ECHO OFF
REM Shutdown Framework Demo - Build Script
REM Dependencies: shutdown, console, memory, file, string, async

gcc --std=c17 -Os -I ../../include demo.c ^
    ../../src/shutdown/shutdown.c ^
    ../../src/console/console.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../arch/file/windows-amd64/fileWrite.c ^
    ../../arch/file/windows-amd64/fileWriteMmap.c ^
    ../../arch/file/windows-amd64/fileWriteRing.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../arch/shutdown/windows-amd64/atomic.c ^
    ../../arch/signals/windows-amd64/signal.c ^
    -lkernel32 ^
    -o demo.exe

strip --strip-unneeded demo.exe
