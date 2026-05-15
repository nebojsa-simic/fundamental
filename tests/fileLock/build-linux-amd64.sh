#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/file/linux-amd64/fileLock.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/async/async.c \
    ../../arch/async/linux-amd64/async.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileReadRing.c \
    -o test

strip --strip-unneeded test