#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../../include \
    test.c \
    -o test

strip --strip-unneeded test
