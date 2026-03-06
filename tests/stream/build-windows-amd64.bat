@ECHO OFF

REM Compile
gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    testFileStreamError.c ^
    testFileStreamLifecycle.c ^
    testFileStreamRead.c ^
    testFileStreamAdvanced.c ^
    ../../src/stream/streamFile.c ^
    ../../src/stream/streamLifecycle.c ^
    ../../src/stream/streamFlow.c ^
    ../../arch/stream/windows-amd64/streamOpen.c ^
    ../../arch/stream/windows-amd64/streamRead.c ^
    ../../arch/file/windows-amd64/fileReadMmap.c ^
    ../../arch/file/windows-amd64/fileRead.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/async/async.c ^
    -o test.exe

REM Strip unnecessary symbols
strip --strip-unneeded test.exe