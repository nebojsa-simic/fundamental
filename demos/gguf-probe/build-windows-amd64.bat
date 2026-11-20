@echo off
gcc --std=c17 -Os -I ../../include ^
    main.c ^
    ../../src/gguf/gguf.c ^
    ../../src/gguf/gguf_dequant.c ^
    ../../src/math/math_scalar.c ^
    ../../arch/gguf/windows-amd64/mmap.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringTemplate.c ^
    -o gguf-probe.exe
if %ERRORLEVEL% EQU 0 echo Build complete: gguf-probe.exe