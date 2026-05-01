#!/bin/bash
# Build script for path_type tests on Linux AMD64

set -e

echo "Building path_type tests..."

# Set up paths
SRC_DIR="../../src"
INCLUDE_DIR="../../include"
ARCH_DIR="../../arch"

# Compiler flags
CC=gcc
CFLAGS="-Wall -Wextra -I${INCLUDE_DIR}"

# Common sources
COMMON_SRCS="\
${SRC_DIR}/filesystem/path.c \
${SRC_DIR}/filesystem/directory.c \
${ARCH_DIR}/memory/windows-amd64/memory.c \
${SRC_DIR}/string/stringOperations.c \
${SRC_DIR}/string/stringValidation.c \
${ARCH_DIR}/filesystem/windows-amd64/directory.c \
${ARCH_DIR}/filesystem/windows-amd64/path.c"

# Compile test_path_conversion.c
echo "Compiling test_path_conversion.c..."
${CC} ${CFLAGS} \
   test_path_conversion.c \
   $COMMON_SRCS \
   -o test_conversion

# Compile test_path_operations.c
echo "Compiling test_path_operations.c..."
${CC} ${CFLAGS} \
   test_path_operations.c \
   $COMMON_SRCS \
   -o test_operations

# Compile test_path_component_access.c
echo "Compiling test_path_component_access.c..."
${CC} ${CFLAGS} \
   test_path_component_access.c \
   $COMMON_SRCS \
   -o test_component_access

# Create wrapper test script
cat > test << 'EOF'
#!/bin/bash
set -e
cd "$(dirname "$0")"
./test_conversion
./test_operations
./test_component_access
echo "All path_type tests passed!"
EOF
chmod +x test

echo "Build complete!"
