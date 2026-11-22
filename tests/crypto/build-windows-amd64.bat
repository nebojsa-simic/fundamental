@ECHO OFF

REM Compile crypto tests
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test_crypto.c ^
    ../../src/crypto/sha256.c ^
    ../../src/crypto/hmac.c ^
    ../../src/crypto/aes.c ^
    ../../src/crypto/aes_gcm.c ^
    ../../src/crypto/ec_p256.c ^
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