#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    -I ../../arch/file/windows-amd64 \
    test.c \
    ../../arch/file/windows-amd64/fileRead.c \
    ../../arch/file/windows-amd64/fileReadMmap.c \
    ../../arch/file/windows-amd64/fileReadRing.c \
    ../../arch/memory/windows-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/async/async.c \
    ../../arch/async/windows-amd64/async.c \
    -o test.exe

strip --strip-unneeded test.exe
