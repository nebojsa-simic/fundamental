#!/bin/bash

gcc \
    --std=c17 -Os \
    -I ../../include \
    -D FUNDAMENTAL_LOG_LEVEL=LOG_LEVEL_TRACE \
    -D FUNDAMENTAL_LOG_OUTPUT_CONSOLE=1 \
    -D FUNDAMENTAL_LOG_OUTPUT_FILE=0 \
    -D FUNDAMENTAL_LOG_BUFFER_SIZE=512 \
    test.c \
    ../../src/logging/logging.c \
    ../../arch/logging/linux-amd64/logging.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    ../../src/string/stringValidation.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o test
