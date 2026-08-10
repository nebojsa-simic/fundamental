@ECHO OFF

REM Compile
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../arch/file/windows-amd64/fileAppend.c ^
    ../../arch/file/windows-amd64/fileRead.c ^
    ../../arch/file/windows-amd64/fileReadMmap.c ^
    ../../arch/file/windows-amd64/fileReadRing.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/async/async.c ^
    ../../arch/async/windows-amd64/async.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    -lkernel32 -ladvapi32 ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe
