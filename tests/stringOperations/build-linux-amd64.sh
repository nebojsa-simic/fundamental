#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/windows-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringValidation.c \
    -o test.exe

strip --strip-unneeded test.exe