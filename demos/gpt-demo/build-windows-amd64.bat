@echo off
gcc -c --std=c17 -Os -mavx2 -mfma -I ../../include model.c -o model.o

gcc -c --std=c17 -Os -mavx2 -mfma -I ../../include ^
    ../../arch/math/avx2/vector.c -o vector_avx2.o

gcc --std=c17 -Os -mavx2 -mfma -I ../../include ^
    main.c tokenizer.c ^
    arch/timing/windows-amd64/timing.c ^
    model.o ^
    ../../src/gguf/gguf.c ^
    ../../src/gguf/gguf_dequant.c ^
    ../../arch/gguf/windows-amd64/mmap.c ^
    ../../src/math/math_scalar.c ^
    ../../src/math/math_init.c ^
    ../../arch/math/windows-amd64/cpu_features.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringTemplate.c ^
    vector_avx2.o ^
    -lm -o demo.exe
if %ERRORLEVEL% EQU 0 echo Build complete: demo.exe
