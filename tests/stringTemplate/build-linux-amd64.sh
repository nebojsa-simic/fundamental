#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/windows-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    -o test.exe

strip --strip-unneeded test.exe