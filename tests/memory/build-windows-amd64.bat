@ECHO OFF

REM Compile
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -o test.exe 

REM Strip unnecessary symbols
strip --strip-unneeded test.exe