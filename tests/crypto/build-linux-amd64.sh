#!/bin/bash
# Build crypto tests - Linux AMD64
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR/../.."
gcc --std=c17 -Os \
    -I "$ROOT/include" \
    "$SCRIPT_DIR/test_crypto.c" \
    "$ROOT/src/crypto/sha256.c" \
    "$ROOT/src/crypto/hmac.c" \
    "$ROOT/src/crypto/aes.c" \
    "$ROOT/src/crypto/aes_gcm.c" \
    "$ROOT/src/crypto/ec_p256.c" \
    "$ROOT/src/bignumber/bignumber.c" \
    "$ROOT/arch/memory/linux-amd64/memory.c" \
    "$ROOT/src/console/console.c" \
    "$ROOT/arch/console/linux-amd64/console.c" \
    "$ROOT/src/string/stringConversion.c" \
    "$ROOT/src/string/stringOperations.c" \
    -o "$SCRIPT_DIR/test"
echo "Build successful!"