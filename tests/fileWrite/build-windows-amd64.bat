@ECHO OFF

REM Compile
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../arch/file/windows-amd64/fileWriteMmap.c ^
    ../../arch/file/windows-amd64/fileWriteRing.c ^
    ../../arch/file/windows-amd64/fileWrite.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/async/async.c ^
    ../../arch/async/windows-amd64/async.c ^
    -lkernel32 ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe
