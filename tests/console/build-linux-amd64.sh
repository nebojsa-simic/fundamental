#!/bin/bash

gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    ../../src/string/stringValidation.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o test
