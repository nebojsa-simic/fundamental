#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test_linux.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/async/async.c \
    -o test

strip --strip-unneeded test