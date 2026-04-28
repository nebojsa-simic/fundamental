#!/bin/bash
# Build script for startup tests - Linux AMD64

set -e

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Compiler
CC=gcc

# Compiler flags
CFLAGS="-I$PROJECT_ROOT/include -Wall -Wextra -g -O0 -DFUNDAMENTAL_STARTUP_VERBOSE=1"

# Source files
SOURCES="
    $SCRIPT_DIR/test.c
    $PROJECT_ROOT/src/startup/startup.c
    $PROJECT_ROOT/arch/startup/linux-amd64/linux.c
    $PROJECT_ROOT/src/platform/platform.c
    $PROJECT_ROOT/arch/platform/linux-amd64/platform.c
    $PROJECT_ROOT/src/filesystem/path.c
    $PROJECT_ROOT/src/filesystem/file_exists.c
    $PROJECT_ROOT/src/filesystem/file_size.c
    $PROJECT_ROOT/src/filesystem/directory.c
    $PROJECT_ROOT/arch/filesystem/linux-amd64/path.c
    $PROJECT_ROOT/arch/filesystem/linux-amd64/file_exists.c
    $PROJECT_ROOT/arch/filesystem/linux-amd64/file_size.c
    $PROJECT_ROOT/arch/filesystem/linux-amd64/directory.c
    $PROJECT_ROOT/src/config/config.c
    $PROJECT_ROOT/src/config/iniParser.c
    $PROJECT_ROOT/src/config/cliParser.c
    $PROJECT_ROOT/arch/config/linux-amd64/env.c
    $PROJECT_ROOT/src/network/network.c
    $PROJECT_ROOT/arch/network/linux-amd64/network.c
    $PROJECT_ROOT/src/string/stringOperations.c
    $PROJECT_ROOT/src/string/stringConversion.c
    $PROJECT_ROOT/src/string/stringTemplate.c
    $PROJECT_ROOT/src/hashmap/hashmap.c
    $PROJECT_ROOT/src/console/console.c
    $PROJECT_ROOT/arch/memory/linux-amd64/memory.c
    $PROJECT_ROOT/arch/console/linux-amd64/console.c
"

# Build
echo "Building startup tests..."
$CC $CFLAGS $SOURCES -o "$SCRIPT_DIR/test"

echo "Build successful!"
