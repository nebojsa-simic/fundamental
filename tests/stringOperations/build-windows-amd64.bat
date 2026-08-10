@ECHO OFF

REM Compile
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringValidation.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringConversion.c ^
    -o test.exe 

REM Strip unnecessary symbols
strip --strip-unneeded test.exe