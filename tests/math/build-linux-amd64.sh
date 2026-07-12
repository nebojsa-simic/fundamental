#!/bin/bash
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
    -lm \
    -o test

if [ $? -eq 0 ]; then echo "Build complete: test"; fi
