@ECHO OFF

REM Compile
gcc ^
    --std=c17 -Os ^
    test.c ^
    ../../src/memory/windows-amd64/memory.c ^
    -o test.exe 

REM Strip unnecessary symbols
strip --strip-unneeded test.exe