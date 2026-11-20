#!/bin/bash
# Parallel Trace Demo - Build Script (freestanding)
# Dependencies: trace, tls, timing, thread_pool, sync, string, console, memory, startup
# Built with -nostdlib so no libc/libgcc/CRT is linked. If trace.c
# reintroduces _Thread_local/__thread, this fails with
# "undefined reference to __emutls_get_address".
gcc --std=c17 -Os -nostdlib -ffreestanding -I ../../include \
    demo.c \
    ../../arch/startup/linux-amd64/linux.c \
    ../../arch/config/linux-amd64/env.c \
    ../../src/trace/trace.c \
    ../../arch/timing/linux-amd64/timing.c \
    ../../arch/tls/linux-amd64/tls.c \
    ../../src/thread_pool/thread_pool.c \
    ../../arch/thread_pool/linux-amd64/thread_pool.c \
    ../../arch/sync/linux-amd64/sync.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    -o demo

if [ $? -ne 0 ]; then
    echo ""
    echo "[ERROR] Build failed."
    echo "        If the log shows 'undefined reference to __emutls_get_address',"
    echo "        src/trace/trace.c is using emulated TLS. Route thread-local"
    echo "        storage through arch/tls/ instead of _Thread_local/__thread."
    exit 1
fi

strip --strip-unneeded demo
echo "Build complete: demo"
