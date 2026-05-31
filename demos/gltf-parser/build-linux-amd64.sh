#!/bin/bash
# glTF Parser Demo — Build Script
# Dependencies: file, stream, string, console, async, memory, json

gcc --std=c17 -Os -I ../../include demo.c \
    ../../src/json/tokenizer.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileReadRing.c \
    ../../src/stream/streamFile.c \
    ../../src/stream/streamLifecycle_linux.c \
    ../../src/stream/streamFlow.c \
    ../../arch/stream/linux-amd64/streamOpen.c \
    ../../arch/stream/linux-amd64/streamRead.c \
    ../../arch/stream/linux-amd64/streamWrite.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringValidation.c \
    ../../src/string/stringTemplate.c \
    ../../src/console/console.c \
    ../../src/async/async.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/async/linux-amd64/async.c \
    ../../arch/config/linux-amd64/env.c \
    -o demo
