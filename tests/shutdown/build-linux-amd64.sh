#!/bin/bash
# Shutdown Framework Tests - Build Script
# Dependencies: shutdown, console, memory, atomic, string

gcc --std=c17 -Os -I ../../include test.c \
    ../../src/shutdown/shutdown.c \
    ../../src/console/console.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/shutdown/linux-amd64/atomic.c \
    ../../arch/signals/linux-amd64/signal.c \
    -o test
