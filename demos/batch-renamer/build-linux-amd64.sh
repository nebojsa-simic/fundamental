#!/bin/bash
# Batch Renamer Demo - Build Script
# Dependencies: filesystem, string, console, array, memory

gcc --std=c17 -Os -I ../../include demo.c \
    ../../src/filesystem/directory.c \
    ../../src/filesystem/path.c \
    ../../src/filesystem/file_exists.c \
    ../../src/filesystem/file_size.c \
    ../../src/filesystem/walk.c \
    ../../src/tsv/tsv.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/string/stringValidation.c \
    ../../src/console/console.c \
    ../../src/array/array.c \
    ../../arch/filesystem/linux-amd64/directory.c \
    ../../arch/filesystem/linux-amd64/path.c \
    ../../arch/filesystem/linux-amd64/file_exists.c \
    ../../arch/filesystem/linux-amd64/file_size.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o demo
