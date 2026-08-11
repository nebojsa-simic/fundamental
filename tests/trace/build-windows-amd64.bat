@ECHO OFF

gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/trace/trace.c ^
    ../../arch/timing/windows-amd64/timing.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    -o test.exe

strip --strip-unneeded test.exe
