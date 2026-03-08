#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringValidation.c \
    ../../src/string/stringConversion.c \
    -o test

strip --strip-unneeded test