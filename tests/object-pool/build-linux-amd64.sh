#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../src/object-pool/object-pool.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o test

strip --strip-unneeded test
