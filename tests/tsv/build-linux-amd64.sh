#!/bin/bash
# Build script for tsv module tests - Linux AMD64

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../.."

# Compiler flags
CC=gcc
CFLAGS="-I$PROJECT_ROOT/include -Wall -Wextra -g -O0"

# Source files
SOURCES=(
    "$SCRIPT_DIR/test_tsv.c"
    "$PROJECT_ROOT/src/tsv/tsv.c"
    "$PROJECT_ROOT/src/string/stringOperations.c"
    "$PROJECT_ROOT/src/string/stringValidation.c"
    "$PROJECT_ROOT/arch/memory/windows-amd64/memory.c"
)

# Build
echo "Building tsv tests..."
$CC $CFLAGS "${SOURCES[@]}" -o "$SCRIPT_DIR/test"

echo "Build successful!"
