#!/bin/bash
# Build script for network module tests - Linux AMD64

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../.."

# Compiler flags
CC=gcc
CFLAGS="-I$PROJECT_ROOT/include -Wall -Wextra -g -O0"

# Source files
SOURCES=(
    "$SCRIPT_DIR/test_network.c"
    "$PROJECT_ROOT/src/network/network.c"
    "$PROJECT_ROOT/arch/network/linux-amd64/network.c"
    "$PROJECT_ROOT/src/async/async.c"
    "$PROJECT_ROOT/arch/memory/linux-amd64/memory.c"
    "$PROJECT_ROOT/src/string/stringOperations.c"
    "$PROJECT_ROOT/src/string/stringValidation.c"
)

# Build
echo "Building network tests..."
$CC $CFLAGS "${SOURCES[@]}" -o "$SCRIPT_DIR/test"

echo "Build successful!"
