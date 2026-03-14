@ECHO OFF

REM Compile main test suite
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    testFileStreamError.c ^
    testFileStreamLifecycle.c ^
    testFileStreamRead.c ^
    testFileStreamWrite.c ^
    testStreamCanWrite.c ^
    testFileStreamAdvanced.c ^
    ../../src/stream/streamFile.c ^
    ../../src/stream/streamLifecycle.c ^
    ../../src/stream/streamFlow.c ^
    ../../arch/stream/windows-amd64/streamOpen.c ^
    ../../arch/stream/windows-amd64/streamRead.c ^
    ../../arch/stream/windows-amd64/streamWrite.c ^
    ../../arch/file/windows-amd64/fileReadMmap.c ^
    ../../arch/file/windows-amd64/fileReadRing.c ^
    ../../arch/file/windows-amd64/fileRead.c ^
    -lkernel32 ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/async/async.c ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe

echo Stream module tests built successfully!
echo All tests including write functionality are now included.