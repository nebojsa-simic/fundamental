@ECHO OFF
REM Log Analyzer Demo - Build Script
REM Dependencies: file (read), string, console, hashmap, memory, async

gcc --std=c17 -Os -I ../../include demo.c ^
    ../../arch/file/windows-amd64/fileRead.c ^
    ../../arch/file/windows-amd64/fileReadMmap.c ^
    ../../arch/file/windows-amd64/fileReadRing.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/string/stringValidation.c ^
    ../../src/console/console.c ^
    ../../src/hashmap/hashmap.c ^
    ../../src/async/async.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../arch/async/windows-amd64/async.c ^
    -lkernel32 ^
    -o demo.exe

strip --strip-unneeded demo.exe
