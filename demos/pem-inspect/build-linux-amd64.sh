#!/bin/bash
# pem-inspect demo - Linux AMD64 build
# Dependencies: crypto (PEM/X.509/SHA-256/EC), file (read), filesystem
#               (path, file_size), async, console, memory, string, timing
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR/../.."
gcc --std=c17 -Os \
    -I "$ROOT/include" \
    "$SCRIPT_DIR/demo.c" \
    "$ROOT/src/crypto/pem.c" \
    "$ROOT/src/crypto/x509.c" \
    "$ROOT/src/crypto/sha256.c" \
    "$ROOT/src/crypto/ec_p256.c" \
    "$ROOT/src/bignumber/bignumber.c" \
    "$ROOT/arch/timing/linux-amd64/timing.c" \
    "$ROOT/src/filesystem/path.c" \
    "$ROOT/src/filesystem/file_size.c" \
    "$ROOT/arch/filesystem/linux-amd64/path.c" \
    "$ROOT/arch/filesystem/linux-amd64/file_size.c" \
    "$ROOT/arch/file/linux-amd64/fileRead.c" \
    "$ROOT/arch/file/linux-amd64/fileReadMmap.c" \
    "$ROOT/arch/file/linux-amd64/fileReadRing.c" \
    "$ROOT/src/async/async.c" \
    "$ROOT/arch/async/linux-amd64/async.c" \
    "$ROOT/src/console/console.c" \
    "$ROOT/arch/console/linux-amd64/console.c" \
    "$ROOT/src/string/stringOperations.c" \
    "$ROOT/src/string/stringConversion.c" \
    "$ROOT/src/string/stringTemplate.c" \
    "$ROOT/arch/memory/linux-amd64/memory.c" \
    -o "$SCRIPT_DIR/pem-inspect"
echo "Build successful!"