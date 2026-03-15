#!/bin/bash

gcc \
    --std=c17 -Os \
    -I ../../include \
    test_process.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/async/async.c \
    ../../src/process/process.c \
    ../../arch/process/linux-amd64/process.c \
    ../../src/string/stringOperations.c \
    -o test

strip --strip-unneeded test
