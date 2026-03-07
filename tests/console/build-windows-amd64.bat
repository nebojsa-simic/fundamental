@ECHO OFF

REM Compile console module tests
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/string/stringValidation.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe
