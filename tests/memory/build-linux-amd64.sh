#!/bin/sh
gcc \
    --std=c17 -Os \
    test.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o test 

strip --strip-unneeded test