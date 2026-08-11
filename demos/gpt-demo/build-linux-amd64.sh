#!/bin/bash
gcc -c --std=c17 -Os -mavx2 -mfma -I ../../include model.c -o model.o

gcc -c --std=c17 -Os -mavx2 -mfma -I ../../include \
    ../../arch/math/avx2/vector.c -o vector_avx2.o

gcc --std=c17 -Os -mavx2 -mfma -I ../../include \
    main.c tokenizer.c \
    arch/timing/linux-amd64/timing.c \
    model.o \
    ../../src/compute/compute_graph.c \
    ../../src/thread_pool/thread_pool.c \
    ../../arch/thread_pool/linux-amd64/thread_pool.c \
    ../../arch/sync/linux-amd64/sync.c \
    ../../src/gguf/gguf.c \
    ../../src/gguf/gguf_dequant.c \
    ../../arch/gguf/linux-amd64/mmap.c \
    ../../src/math/math_scalar.c \
    ../../src/math/math_init.c \
    ../../arch/math/linux-amd64/cpu_features.c \
    ../../src/console/console.c \
    ../../arch/console/linux-amd64/console.c \
    ../../arch/memory/linux-amd64/memory.c \
    ../../src/string/stringConversion.c \
    ../../src/string/stringOperations.c \
    ../../src/string/stringTemplate.c \
    vector_avx2.o \
    -lm -o demo

if [ $? -eq 0 ]; then echo "Build complete: demo"; fi
