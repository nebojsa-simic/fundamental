#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/async/async.c \
    ../../arch/async/linux-amd64/async.c \
    -o test 

strip --strip-unneeded test