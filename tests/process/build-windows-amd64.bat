@ECHO OFF

REM Compile
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test_process.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/async/async.c ^
    ../../arch/async/windows-amd64/async.c ^
    ../../src/process/process.c ^
    ../../arch/process/windows-amd64/process.c ^
    ../../src/string/stringOperations.c ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe
