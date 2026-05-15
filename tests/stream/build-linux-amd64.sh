#!/bin/sh
# Compile main test suite - includes all stream tests (read, write, lifecycle, etc.)
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    testFileStreamError.c \
    testFileStreamLifecycle.c \
    testFileStreamRead.c \
    testFileStreamWrite.c \
    testStreamCanWrite.c \
    testFileStreamAdvanced.c \
    ../../src/stream/streamFile.c \
    ../../src/stream/streamLifecycle_linux.c \
    ../../src/stream/streamFlow.c \
    ../../arch/stream/linux-amd64/streamOpen.c \
    ../../arch/stream/linux-amd64/streamRead.c \
    ../../arch/stream/linux-amd64/streamWrite.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/file/linux-amd64/fileReadRing.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/async/async.c \
    ../../arch/async/linux-amd64/async.c \
    -o test 

strip --strip-unneeded test

echo "Stream module tests built successfully!"
echo "All tests including write functionality are now included."