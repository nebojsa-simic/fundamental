#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../../include \
    test.c \
    ../../../arch/file/windows-amd64/fileRead.c \
    ../../../arch/file/windows-amd64/fileReadMmap.c \
    ../../../arch/file/windows-amd64/fileWrite.c \
    ../../../arch/file/windows-amd64/fileWriteMmap.c \
    ../../../arch/file/windows-amd64/fileAppend.c \
    ../../../arch/file/windows-amd64/fileLock.c \
    ../../../src/string/stringOperations.c \
    ../../../src/string/stringConversion.c \
    ../../../src/string/stringTemplate.c \
    ../../../src/async/async.c \
    ../../../arch/async/windows-amd64/async.c \
    ../../../arch/memory/windows-amd64/memory.c \
    -lpthread \
    -o test.exe

strip --strip-unneeded test.exe
