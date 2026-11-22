@ECHO OFF
REM INI config end-to-end demo - Build Script
REM Dependencies: config, hashmap, filesystem, console, string, memory
gcc --std=c17 -Os -nostdlib -ffreestanding -mno-stack-arg-probe ^
    -Wl,--subsystem,console ^
    -I ../../include ^
    demo.c ^
    ../../arch/startup/windows-amd64/windows.c ^
    ../../src/config/config.c ^
    ../../src/config/iniParser.c ^
    ../../src/config/cliParser.c ^
    ../../arch/config/windows-amd64/env.c ^
    ../../src/hashmap/hashmap.c ^
    ../../src/filesystem/path.c ^
    ../../src/filesystem/file_exists.c ^
    ../../arch/filesystem/windows-amd64/path.c ^
    ../../arch/filesystem/windows-amd64/file_exists.c ^
    ../../arch/filesystem/windows-amd64/directory.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringValidation.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -lkernel32 ^
    -o demo.exe
strip --strip-unneeded demo.exe
