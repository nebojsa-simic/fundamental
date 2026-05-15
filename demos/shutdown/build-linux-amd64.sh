#!/bin/bash
# Shutdown Framework Demo - Build Script
# Dependencies: shutdown, console, memory, file, string, async, startup

gcc --std=c17 -Os -I ../../include demo.c \
    ../../src/shutdown/shutdown.c \
    ../../src/startup/startup.c \
    ../../src/console/console.c \
    ../../src/async/async.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/platform/platform.c \
    ../../src/filesystem/path.c \
    ../../src/filesystem/file_exists.c \
    ../../src/filesystem/directory.c \
    ../../src/config/config.c \
    ../../src/config/iniParser.c \
    ../../src/config/cliParser.c \
    ../../src/hashmap/hashmap.c \
    ../../arch/file/linux-amd64/fileWrite.c \
    ../../arch/file/linux-amd64/fileWriteMmap.c \
    ../../arch/file/linux-amd64/fileWriteRing.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/shutdown/linux-amd64/atomic.c \
    ../../arch/signals/linux-amd64/signal.c \
    ../../arch/async/linux-amd64/async.c \
    ../../arch/platform/linux-amd64/platform.c \
    ../../arch/filesystem/linux-amd64/path.c \
    ../../arch/filesystem/linux-amd64/file_exists.c \
    ../../arch/filesystem/linux-amd64/directory.c \
    ../../arch/config/linux-amd64/env.c \
    -o demo
