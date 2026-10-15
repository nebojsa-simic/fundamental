#!/bin/bash
gcc -c --std=c17 -Os -I ../../include model.c -o model.o

gcc --std=c17 -Os -I ../../include \
    main.c tokenizer.c \
    model.o \
    ../../src/compute/compute_graph.c \
    ../../src/thread_pool/thread_pool.c \
    ../../arch/thread_pool/linux-amd64/thread_pool.c \
    ../../arch/sync/linux-amd64/sync.c \
    ../../src/trace/trace.c \
    ../../arch/timing/linux-amd64/timing.c \
    ../../src/gguf/gguf.c \
    ../../src/gguf/gguf_dequant.c \
    ../../arch/gguf/linux-amd64/mmap.c \
    ../../src/math/math_scalar.c \
    ../../src/math/math_init.c \
    ../../arch/math/scalar/vector.c \
    ../../arch/math/scalar/vector_mxfp4.c \
    ../../arch/math/linux-amd64/cpu_features.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    ../../src/stream/streamFile.c \
    ../../src/stream/streamFlow.c \
    ../../src/stream/streamLifecycle.c \
    ../../arch/stream/linux-amd64/streamOpen.c \
    ../../arch/stream/linux-amd64/streamRead.c \
    ../../arch/stream/linux-amd64/streamWrite.c \
    ../../arch/file/linux-amd64/fileRead.c \
    ../../arch/file/linux-amd64/fileReadMmap.c \
    ../../arch/file/linux-amd64/fileReadRing.c \
    ../../src/async/async.c \
    ../../arch/async/linux-amd64/async.c \
    -lm -o demo

if [ $? -eq 0 ]; then echo "Build complete: demo"; fi
