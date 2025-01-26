#!/bin/sh
gcc \
    --std=c17 -Os \
    test.c \
    ../../src/memory/linux-amd64/memory.c \
    -o test 

strip --strip-unneeded test