#!/bin/bash
# Minimal Logging Demo - Build Script
# Dependencies: logging, console, string (all 4 files), memory
gcc --std=c17 -Os \
    -I ../../include \
    -D FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_DEBUG \
    -D FUNDAMENTAL_LOG_OUTPUT_CONSOLE=1 \
    -D FUNDAMENTAL_LOG_OUTPUT_FILE=0 \
    demo.c \
    ../../src/logging/logging.c \
    ../../src/console/console.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    ../../src/string/stringValidation.c \
    ../../arch/logging/linux-amd64/logging.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o demo
