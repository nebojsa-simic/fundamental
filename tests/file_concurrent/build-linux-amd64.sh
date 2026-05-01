#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/file/windows-amd64/fileLock.c \
    ../../arch/memory/windows-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/async/async.c \
    ../../arch/async/windows-amd64/async.c \
    -o test.exe

strip --strip-unneeded test.exe
