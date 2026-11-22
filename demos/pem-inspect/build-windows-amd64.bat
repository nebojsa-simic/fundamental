@ECHO OFF

REM pem-inspect demo — Windows AMD64 build
REM Dependencies: crypto (PEM/X.509/SHA-256/EC), file (read), filesystem
REM               (path, file_size), async, console, memory, string, timing

gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    demo.c ^
    ../../src/crypto/pem.c ^
    ../../src/crypto/x509.c ^
    ../../src/crypto/sha256.c ^
    ../../src/crypto/ec_p256.c ^
    ../../src/bignumber/bignumber.c ^
    ../../arch/timing/windows-amd64/timing.c ^
    ../../src/filesystem/path.c ^
    ../../src/filesystem/file_size.c ^
    ../../arch/filesystem/windows-amd64/path.c ^
    ../../arch/filesystem/windows-amd64/file_size.c ^
    ../../arch/file/windows-amd64/fileRead.c ^
    ../../arch/file/windows-amd64/fileReadMmap.c ^
    ../../arch/file/windows-amd64/fileReadRing.c ^
    ../../src/async/async.c ^
    ../../arch/async/windows-amd64/async.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -lkernel32 ^
    -o pem-inspect.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful! Run: pem-inspect.exe [file.pem]
exit /b 0