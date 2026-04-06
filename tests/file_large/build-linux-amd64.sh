#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../../include \
    test.c \
    ../../../arch/file/linux-amd64/fileRead.c \
    ../../../arch/file/linux-amd64/fileReadMmap.c \
    ../../../arch/file/linux-amd64/fileWrite.c \
    ../../../arch/file/linux-amd64/fileWriteMmap.c \
    ../../../arch/file/linux-amd64/fileAppend.c \
    ../../../arch/file/linux-amd64/fileLock.c \
    ../../../src/string/stringOperations.c \
    ../../../src/string/stringConversion.c \
    ../../../src/string/stringTemplate.c \
    ../../../src/async/async.c \
    ../../../arch/async/linux-amd64/async.c \
    ../../../arch/memory/linux-amd64/memory.c \
    -lpthread \
    -o test

strip --strip-unneeded test
