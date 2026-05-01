#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/file/windows-amd64/fileAppend.c \
    ../../arch/memory/windows-amd64/memory.c \
    ../../src/async/async.c \
    ../../arch/async/windows-amd64/async.c \
    ../../arch/file/windows-amd64/fileRead.c \
    ../../arch/file/windows-amd64/fileReadMmap.c \
    ../../arch/file/windows-amd64/fileReadRing.c \
    -o test.exe

strip --strip-unneeded test.exe