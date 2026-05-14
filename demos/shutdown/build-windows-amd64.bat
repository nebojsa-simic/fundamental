@ECHO OFF
REM Shutdown Framework Demo - Build Script
REM Dependencies: shutdown, console, memory, file, string, async, startup

gcc --std=c17 -Os -I ../../include demo.c ^
    ../../src/shutdown/shutdown.c ^
    ../../src/startup/startup.c ^
    ../../src/platform/platform.c ^
    ../../src/filesystem/path.c ^
    ../../src/filesystem/file_exists.c ^
    ../../src/config/config.c ^
    ../../src/config/iniParser.c ^
    ../../src/config/cliParser.c ^
    ../../src/hashmap/hashmap.c ^
    ../../src/async/async.c ^
    ../../arch/async/windows-amd64/async.c ^
    ../../src/console/console.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringTemplate.c ^
    ../../arch/file/windows-amd64/fileWrite.c ^
    ../../arch/file/windows-amd64/fileWriteMmap.c ^
    ../../arch/file/windows-amd64/fileWriteRing.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../arch/shutdown/windows-amd64/atomic.c ^
    ../../arch/signals/windows-amd64/signal.c ^
    ../../arch/platform/windows-amd64/platform.c ^
    ../../arch/filesystem/windows-amd64/path.c ^
    ../../arch/filesystem/windows-amd64/file_exists.c ^
    ../../arch/filesystem/windows-amd64/directory.c ^
    ../../arch/config/windows-amd64/env.c ^
    -lkernel32 ^
    -o demo.exe

strip --strip-unneeded demo.exe
