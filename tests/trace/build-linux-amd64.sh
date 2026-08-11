#!/bin/bash

gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/trace/trace.c \
    ../../arch/timing/linux-amd64/timing.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    -o test

strip --strip-unneeded test
