#!/bin/bash
# Build script for file_exists tests - Linux AMD64

set -e

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Compiler
CC=gcc

# Compiler flags
CFLAGS="-I$PROJECT_ROOT/include -I$PROJECT_ROOT/arch/filesystem/linux-amd64 -Wall -Wextra -g -O0"

# Source files
SOURCES="
    $SCRIPT_DIR/test_file_exists.c
    $PROJECT_ROOT/src/filesystem/file_exists.c
    $PROJECT_ROOT/src/filesystem/directory.c
    $PROJECT_ROOT/src/filesystem/path.c
    $PROJECT_ROOT/arch/filesystem/linux-amd64/file_exists.c
    $PROJECT_ROOT/arch/filesystem/linux-amd64/directory.c
    $PROJECT_ROOT/arch/filesystem/linux-amd64/path.c
    $PROJECT_ROOT/arch/memory/linux-amd64/memory.c
    $PROJECT_ROOT/src/string/stringValidation.c
    $PROJECT_ROOT/src/string/stringOperations.c
"

# Build
echo "Building file_exists tests..."
$CC $CFLAGS $SOURCES -o "$SCRIPT_DIR/test"

echo "Build successful!"
