#!/bin/bash
# Validates demo directory before build
# Usage: cd demos/<name> && ../../demos/validate-demo.sh

ERROR_COUNT=0

echo "[INFO] Validating demo directory: $(pwd)"
echo ""

# Check demo.c exists
if [ ! -f demo.c ]; then
    echo "[ERROR] demo.c not found in current directory"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Check build script exists
if [ ! -f build-linux-amd64.sh ]; then
    echo "[ERROR] build-linux-amd64.sh not found"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Extract and verify source files from build script
echo "[INFO] Checking source file references..."
if [ -f build-linux-amd64.sh ]; then
    while IFS= read -r line; do
        for word in $line; do
            if [[ "$word" == *.c ]] && [[ ! "$word" =~ ^# ]]; then
                if [ ! -f "$word" ]; then
                    echo "[ERROR] Source file not found: $word"
                    ERROR_COUNT=$((ERROR_COUNT + 1))
                fi
            fi
        done
    done < build-linux-amd64.sh
fi

# Check for common wrong patterns in demo.c
if [ -f demo.c ]; then
    if grep -q "fun_file_read(" demo.c; then
        echo "[ERROR] Wrong API: fun_file_read() - use fun_read_file_in_memory(Read)"
        echo "[HINT] See include/fundamental/file/file.h"
        ERROR_COUNT=$((ERROR_COUNT + 1))
    fi
    
    if grep -q "src/file/file.c" demo.c; then
        echo "[ERROR] Wrong path: src/file/file.c does not exist"
        echo "[HINT] File I/O is in arch/file/<platform>/"
        ERROR_COUNT=$((ERROR_COUNT + 1))
    fi
    
    if grep -q "arch/filesystem.*/file.c" demo.c; then
        echo "[ERROR] Wrong path: arch/filesystem/.../file.c does not exist"
        echo "[HINT] Use arch/file/<platform>/fileRead.c"
        ERROR_COUNT=$((ERROR_COUNT + 1))
    fi
fi

# Check headers referenced in demo.c exist
if [ -f demo.c ]; then
    grep -oE 'fundamental/[^">]+' demo.c 2>/dev/null | while read -r header; do
        if [ ! -f "../../include/$header" ]; then
            echo "[ERROR] Header not found: ../../include/$header"
            ERROR_COUNT=$((ERROR_COUNT + 1))
        fi
    done
fi

# Summary
echo ""
if [ $ERROR_COUNT -gt 0 ]; then
    echo "========================================================"
    echo "[FAILED] $ERROR_COUNT error(s) found - FIX BEFORE BUILDING"
    echo "========================================================"
    echo ""
    echo "[HINT] Read the checklist in AGENTS.md section:"
    echo "       'Before Writing Demo/Test Code'"
    echo ""
    echo "[HINT] Read existing tests: tests/<module>/build-*.bat"
    echo ""
    echo "[HINT] Read headers: include/fundamental/<module>/<module>.h"
    echo ""
    exit 1
else
    echo "========================================================"
    echo "[OK] Validation passed - ready to build"
    echo "========================================================"
    exit 0
fi
