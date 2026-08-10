@ECHO OFF

gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../arch/sync/windows-amd64/sync.c ^
    ../../src/thread_pool/thread_pool.c ^
    ../../arch/thread_pool/windows-amd64/thread_pool.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    -o test.exe

strip --strip-unneeded test.exe
