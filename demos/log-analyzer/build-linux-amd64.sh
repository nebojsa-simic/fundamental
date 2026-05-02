#!/bin/bash
# Log Analyzer Demo - Build Script
# Dependencies: file (read), string, console, hashmap, memory, async

gcc --std=c17 -Os -I ../../include demo.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileReadRing.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/string/stringValidation.c \
    ../../src/console/console.c \
    ../../src/hashmap/hashmap.c \
    ../../src/async/async.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/async/linux-amd64/async.c \
    -o demo
