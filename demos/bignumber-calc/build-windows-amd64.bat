@ECHO OFF

REM bignumber-calc demo — Windows AMD64 build
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    main.c ^
    ../../src/bignumber/bignumber.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    -o bignumber-calc.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful! Run: bignumber-calc.exe
exit /b 0