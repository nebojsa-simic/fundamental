#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    -I ../../arch/file/linux-amd64 \
    test.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileReadRing.c \
    ../../arch/file/linux-amd64/fileWrite.c \
    ../../arch/file/linux-amd64/fileWriteMmap.c \
    ../../arch/file/linux-amd64/fileWriteRing.c \
    ../../arch/file/linux-amd64/fileAppend.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/async/async.c \
    ../../arch/async/linux-amd64/async.c \
    -o test

strip --strip-unneeded test
