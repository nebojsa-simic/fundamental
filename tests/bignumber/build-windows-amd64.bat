@ECHO OFF

REM Compile bignumber tests
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test_bignumber.c ^
    ../../src/bignumber/bignumber.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    -o test.exe 

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful!
exit /b 0