#!/bin/bash
# Build script for config module tests - Linux AMD64

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../.."

# Compiler flags
CC=gcc
CFLAGS="-I$PROJECT_ROOT/include -Wall -Wextra -g -O0"

# Source files
SOURCES=(
    "$SCRIPT_DIR/test_config.c"
    "$PROJECT_ROOT/src/config/config.c"
    "$PROJECT_ROOT/src/config/iniParser.c"
    "$PROJECT_ROOT/src/config/cliParser.c"
    "$PROJECT_ROOT/arch/config/windows-amd64/env.c"
    "$PROJECT_ROOT/arch/memory/windows-amd64/memory.c"
    "$PROJECT_ROOT/src/hashmap/hashmap.c"
    "$PROJECT_ROOT/src/string/stringOperations.c"
    "$PROJECT_ROOT/src/string/stringValidation.c"
    "$PROJECT_ROOT/src/filesystem/path.c"
    "$PROJECT_ROOT/arch/filesystem/windows-amd64/path.c"
    "$PROJECT_ROOT/arch/filesystem/windows-amd64/file_exists.c"
    "$PROJECT_ROOT/src/filesystem/file_exists.c"
    "$PROJECT_ROOT/arch/filesystem/windows-amd64/directory.c"
)

# Build
echo "Building config tests..."
$CC $CFLAGS "${SOURCES[@]}" -o "$SCRIPT_DIR/test"

echo "Build successful!"
