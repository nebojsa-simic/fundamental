@echo off
gcc -c --std=c17 -Os -mavx512f -mavx512bw -mavx512dq -mavx512vl -mfma -I ../../include model.c -o model.o

gcc -c --std=c17 -Os -mavx512f -mavx512bw -mavx512dq -mavx512vl -mfma -I ../../include ^
    ../../arch/math/avx512/vector.c -o vector_avx512.o

gcc -c --std=c17 -Os -mavx2 -mfma -I ../../include ^
    ../../arch/math/avx2/vector.c -o vector_avx2.o

gcc --std=c17 -Os -mavx512f -mavx512bw -mavx512dq -mavx512vl -mfma -I ../../include ^
    main.c tokenizer.c ^
    model.o ^
    ../../src/compute/compute_graph.c ^
    ../../src/thread_pool/thread_pool.c ^
    ../../arch/thread_pool/windows-amd64/thread_pool.c ^
    ../../arch/sync/windows-amd64/sync.c ^
    ../../src/trace/trace.c ^
    ../../arch/timing/windows-amd64/timing.c ^
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
    ../../src/stream/streamFile.c ^
    ../../src/stream/streamFlow.c ^
    ../../src/stream/streamLifecycle.c ^
    ../../arch/stream/windows-amd64/streamOpen.c ^
    ../../arch/stream/windows-amd64/streamRead.c ^
    ../../arch/stream/windows-amd64/streamWrite.c ^
    ../../arch/file/windows-amd64/fileRead.c ^
    ../../arch/file/windows-amd64/fileReadMmap.c ^
    ../../arch/file/windows-amd64/fileReadRing.c ^
    ../../src/async/async.c ^
    ../../arch/async/windows-amd64/async.c ^
    vector_avx512.o ^
    vector_avx2.o ^
    -lm -o demo.exe
if %ERRORLEVEL% EQU 0 echo Build complete: demo.exe
