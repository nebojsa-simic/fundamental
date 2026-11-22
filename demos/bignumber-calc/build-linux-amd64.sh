#!/bin/bash
# bignumber-calc demo - Linux AMD64 build
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR/../.."
gcc --std=c17 -Os \
    -I "$ROOT/include" \
    "$SCRIPT_DIR/main.c" \
    "$ROOT/src/bignumber/bignumber.c" \
    "$ROOT/arch/memory/linux-amd64/memory.c" \
    "$ROOT/src/console/console.c" \
    "$ROOT/arch/console/linux-amd64/console.c" \
    "$ROOT/src/string/stringConversion.c" \
    "$ROOT/src/string/stringOperations.c" \
    -o "$SCRIPT_DIR/bignumber-calc"
echo "Build successful!"