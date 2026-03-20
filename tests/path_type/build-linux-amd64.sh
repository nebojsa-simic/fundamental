#!/bin/bash
# Build script for path_type tests on Linux AMD64

set -e

echo "Building path_type tests..."

# Set up paths
SRC_DIR="../../src"
INCLUDE_DIR="../../include"
ARCH_DIR="../../arch"

# Compiler flags
CC=gcc
CFLAGS="-Wall -Wextra -I${INCLUDE_DIR} -I${ARCH_DIR}/linux-amd64"

# Compile test_path_conversion.c
echo "Compiling test_path_conversion.c..."
${CC} ${CFLAGS} \
   test_path_conversion.c \
   "${SRC_DIR}/filesystem/path.c" \
   "${SRC_DIR}/filesystem/directory.c" \
   "${SRC_DIR}/memory/memory.c" \
   "${SRC_DIR}/string/string.c" \
   "${SRC_DIR}/error/error.c" \
   "${ARCH_DIR}/linux-amd64/directory.c" \
   "${ARCH_DIR}/linux-amd64/path.c" \
   -o test_conversion

echo "Compiling test_path_operations.c..."
${CC} ${CFLAGS} \
   test_path_operations.c \
   "${SRC_DIR}/filesystem/path.c" \
   "${SRC_DIR}/filesystem/directory.c" \
   "${SRC_DIR}/memory/memory.c" \
   "${SRC_DIR}/string/string.c" \
   "${SRC_DIR}/error/error.c" \
   "${ARCH_DIR}/linux-amd64/directory.c" \
   "${ARCH_DIR}/linux-amd64/path.c" \
   -o test_operations

echo "Compiling test_path_component_access.c..."
${CC} ${CFLAGS} \
   test_path_component_access.c \
   "${SRC_DIR}/filesystem/path.c" \
   "${SRC_DIR}/filesystem/directory.c" \
   "${SRC_DIR}/memory/memory.c" \
   "${SRC_DIR}/string/string.c" \
   "${SRC_DIR}/error/error.c" \
   "${ARCH_DIR}/linux-amd64/directory.c" \
   "${ARCH_DIR}/linux-amd64/path.c" \
   -o test_component_access

echo "Build complete!"
