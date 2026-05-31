#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../.."

CC=gcc
CFLAGS="-I$PROJECT_ROOT/include -Wall -Wextra -g -O0"

SOURCES=(
    "$SCRIPT_DIR/test_json.c"
    "$PROJECT_ROOT/src/json/tokenizer.c"
    "$PROJECT_ROOT/src/string/stringOperations.c"
)

echo "Building json tests..."
$CC $CFLAGS "${SOURCES[@]}" -o "$SCRIPT_DIR/test"

echo "Build successful!"
