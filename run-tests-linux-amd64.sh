#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

for test_dir in tests/*/; do
    if [ -f "$test_dir/build-linux-amd64.sh" ]; then
        echo "Compiling tests in $test_dir"
        cd "$test_dir"
        rm -f ./test
        bash ./build-linux-amd64.sh
        if [ $? -ne 0 ]; then
            echo "Compilation failed in $test_dir"
            exit 1
        fi
        ./test
        cd "$SCRIPT_DIR"
    else
        echo "Skipping $test_dir - no build-linux-amd64.sh found"
    fi
done

echo "All tests completed successfully"