#!/bin/sh
# Compile main test suite
gcc \
    --std=c17 -Os \
    -I ../../include \
    test.c \
    testFileStreamError.c \
    testFileStreamLifecycle.c \
    testFileStreamRead.c \
    testFileStreamAdvanced.c \
    ../../src/stream/streamFile.c \
    ../../src/stream/streamLifecycle_linux.c \
    ../../src/stream/streamFlow.c \
    ../../arch/stream/linux-amd64/streamOpen.c \
    ../../arch/stream/linux-amd64/streamRead.c \
    ../../arch/stream/linux-amd64/streamWrite.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringTemplate.c \
    ../../src/async/async.c \
    -o test 

strip --strip-unneeded test

echo "Main test suite built successfully"
echo ""
echo "Note: Stream write tests are included in main test suite"
echo "testFileStreamWrite.c can be built separately if needed"