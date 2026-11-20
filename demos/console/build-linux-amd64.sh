#!/bin/bash
# Minimal Console Demo - Build Script
# Dependencies: console, string (operations), memory
gcc --std=c17 -Os -nostdlib -ffreestanding -I ../../include demo.c \
    ../../arch/startup/linux-amd64/linux.c \
    ../../arch/config/linux-amd64/env.c \
    ../../src/console/console.c \
    ../../src/string/stringOperations.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o demo
