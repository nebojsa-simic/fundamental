#!/bin/bash
# Build script for stringSubstring tests - Linux AMD64

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../.."

CC=gcc
CFLAGS="-I$PROJECT_ROOT/include -Wall -Wextra -g -O0"

echo "Building test_substring..."
$CC $CFLAGS \
    "$SCRIPT_DIR/test_substring.c" \
    "$PROJECT_ROOT/src/string/stringOperations.c" \
    "$PROJECT_ROOT/src/string/stringValidation.c" \
    -o "$SCRIPT_DIR/test_substring"

echo "Building test_slice..."
$CC $CFLAGS \
    "$SCRIPT_DIR/test_slice.c" \
    "$PROJECT_ROOT/src/string/stringOperations.c" \
    "$PROJECT_ROOT/src/string/stringValidation.c" \
    -o "$SCRIPT_DIR/test_slice"

echo "Build complete!"
