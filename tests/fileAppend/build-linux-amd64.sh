#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/file/linux-amd64/fileAppend.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/async/async.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    -o test

strip --strip-unneeded test