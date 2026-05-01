#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../../include \
    test.c \
    -o test.exe

strip --strip-unneeded test.exe
