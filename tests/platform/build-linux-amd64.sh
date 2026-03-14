#!/bin/bash
# Build script for platform tests - Linux AMD64

set -e

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Compiler
CC=gcc

# Compiler flags
CFLAGS="-I$PROJECT_ROOT/include -Wall -Wextra -g -O0"

# Source files
SOURCES="
    $SCRIPT_DIR/test_platform.c
    $PROJECT_ROOT/src/platform/platform.c
    $PROJECT_ROOT/arch/platform/linux-amd64/platform.c
    $PROJECT_ROOT/src/string/stringOperations.c
    $PROJECT_ROOT/arch/memory/linux-amd64/memory.c
"

# Build
echo "Building platform tests..."
$CC $CFLAGS $SOURCES -o "$SCRIPT_DIR/test"

echo "Build successful!"
