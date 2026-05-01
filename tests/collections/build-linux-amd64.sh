#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../src/array/array.c \
    ../../arch/memory/windows-amd64/memory.c \
    ../../src/async/async.c \
    ../../arch/async/windows-amd64/async.c \
    -o test.exe

strip --strip-unneeded test.exe