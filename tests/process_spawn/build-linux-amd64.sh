#!/bin/bash

gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/async/async.c \
    ../../src/async/process.c \
    ../../arch/async/linux-amd64/process.c \
    -o test
