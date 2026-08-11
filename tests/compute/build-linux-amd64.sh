#!/bin/bash

gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/sync/linux-amd64/sync.c \
    ../../src/thread_pool/thread_pool.c \
    ../../arch/thread_pool/linux-amd64/thread_pool.c \
    ../../src/compute/compute_graph.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    -o test

strip --strip-unneeded test
