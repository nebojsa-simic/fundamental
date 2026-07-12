@echo off
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
    -lm ^
    -o test.exe

if %ERRORLEVEL% EQU 0 echo Build complete: test.exe
