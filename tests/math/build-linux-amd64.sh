#!/bin/bash
gcc -c --std=c17 -Os -mavx2 -mfma -I ../../include ../../arch/math/avx2/vector.c -o vector_avx2.o

gcc \
    --std=c17 -Os \
    -I ../../include \
    test_main.c \
    test_scalar_accuracy.c \
    test_vector_accuracy.c \
    test_edge_cases.c \
    test_consistency.c \
    test_performance.c \
    test_harness_self.c \
    ../../src/math/math_scalar.c \
    ../../src/math/math_init.c \
    ../../arch/math/linux-amd64/cpu_features.c \
    vector_avx2.o \
    -lm \
    -o test

if [ $? -eq 0 ]; then echo "Build complete: test"; fi
