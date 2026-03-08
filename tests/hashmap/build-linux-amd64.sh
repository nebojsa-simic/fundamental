#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../src/hashmap/hashmap.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/async/async.c \
    -o test

strip --strip-unneeded test