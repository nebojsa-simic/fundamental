@ECHO OFF
REM Batch Renamer Demo - Build Script
REM Dependencies: filesystem, string, console, array, memory

gcc --std=c17 -Os -I ../../include demo.c ^
    ../../src/filesystem/directory.c ^
    ../../src/filesystem/path.c ^
    ../../src/filesystem/file_exists.c ^
    ../../src/filesystem/file_size.c ^
    ../../src/filesystem/walk.c ^
    ../../src/tsv/tsv.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/string/stringValidation.c ^
    ../../src/console/console.c ^
    ../../src/array/array.c ^
    ../../arch/filesystem/windows-amd64/directory.c ^
    ../../arch/filesystem/windows-amd64/path.c ^
    ../../arch/filesystem/windows-amd64/file_exists.c ^
    ../../arch/filesystem/windows-amd64/file_size.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -lkernel32 ^
    -o demo.exe

strip --strip-unneeded demo.exe
