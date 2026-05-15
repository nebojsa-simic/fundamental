#!/bin/bash

gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/sync/linux-amd64/sync.c \
    -lpthread \
    -o test

strip --strip-unneeded test
