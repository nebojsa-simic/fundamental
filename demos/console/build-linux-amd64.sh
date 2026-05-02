#!/bin/bash
# Minimal Console Demo - Build Script
# Dependencies: console, string (operations), memory
gcc --std=c17 -Os -I ../../include demo.c \
    ../../src/console/console.c \
    ../../src/string/stringOperations.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o demo
