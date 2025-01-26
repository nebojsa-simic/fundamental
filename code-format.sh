#!/bin/bash
find . -type f \( -name "*.c" -o -name "*.h" \) > files.temp
clang-format --files files.temp -i
rm -f files.temp