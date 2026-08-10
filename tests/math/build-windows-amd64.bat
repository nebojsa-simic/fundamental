@echo off
gcc -c --std=c17 -Os -mavx2 -mfma -I ../../include ../../arch/math/avx2/vector.c -o vector_avx2.o

gcc ^
    --std=c17 -Os ^
    -I ../../include ^
    test_main.c ^
    test_scalar_accuracy.c ^
    test_vector_accuracy.c ^
    test_edge_cases.c ^
    test_consistency.c ^
    test_performance.c ^
    test_harness_self.c ^
    ../../src/math/math_scalar.c ^
    ../../src/math/math_init.c ^
    ../../arch/math/windows-amd64/cpu_features.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    vector_avx2.o ^
    -lm ^
    -o test.exe

if %ERRORLEVEL% EQU 0 echo Build complete: test.exe
