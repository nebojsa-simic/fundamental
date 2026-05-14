#!/bin/bash
# Shutdown Framework Demo - Build Script
# Dependencies: shutdown, console, memory, file, string, async, startup

gcc --std=c17 -Os -I ../../include demo.c \
    ../../src/shutdown/shutdown.c \
    ../../src/startup/startup.c \
    ../../src/console/console.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../arch/file/linux-amd64/fileWrite.c \
    ../../arch/file/linux-amd64/fileWriteMmap.c \
    ../../arch/file/linux-amd64/fileWriteRing.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/shutdown/linux-amd64/atomic.c \
    ../../arch/signals/linux-amd64/signal.c \
    -o demo
