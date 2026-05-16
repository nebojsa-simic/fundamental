#!/bin/sh
gcc \
    --std=c17 -Os \
    -I ../../include \
    reproduce_race.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../arch/sync/linux-amd64/sync.c \
    ../../src/thread_pool/thread_pool.c \
    ../../arch/thread_pool/linux-amd64/thread_pool.c \
    -o reproduce_race
