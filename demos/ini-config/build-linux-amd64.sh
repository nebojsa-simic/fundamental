#!/bin/bash
# INI config end-to-end demo - Build Script
# Dependencies: config, hashmap, filesystem, console, string, memory
set -e
gcc --std=c17 -Os -nostdlib -ffreestanding -I ../../include demo.c \
    ../../arch/startup/linux-amd64/linux.c \
    ../../src/config/config.c \
    ../../src/config/iniParser.c \
    ../../src/config/cliParser.c \
    ../../arch/config/linux-amd64/env.c \
    ../../src/hashmap/hashmap.c \
    ../../src/filesystem/path.c \
    ../../src/filesystem/file_exists.c \
    ../../arch/filesystem/linux-amd64/path.c \
    ../../arch/filesystem/linux-amd64/file_exists.c \
    ../../arch/filesystem/linux-amd64/directory.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringValidation.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o demo
strip --strip-unneeded demo
