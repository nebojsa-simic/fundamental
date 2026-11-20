@ECHO OFF
REM Parallel Trace Demo - Build Script (freestanding)
REM Dependencies: trace, tls, timing, thread_pool, sync, string, console, memory, startup
REM Built with -nostdlib so no libc/libgcc/CRT is linked. If trace.c
REM reintroduces _Thread_local/__thread, this fails with
REM "undefined reference to __emutls_get_address".
gcc --std=c17 -Os -nostdlib -ffreestanding -mno-stack-arg-probe ^
    -Wl,--subsystem,console ^
    -I ../../include ^
    demo.c ^
    ../../arch/startup/windows-amd64/windows.c ^
    ../../src/trace/trace.c ^
    ../../arch/timing/windows-amd64/timing.c ^
    ../../arch/tls/windows-amd64/tls.c ^
    ../../src/thread_pool/thread_pool.c ^
    ../../arch/thread_pool/windows-amd64/thread_pool.c ^
    ../../arch/sync/windows-amd64/sync.c ^
    ../../src/string/stringConversion.c ^
    ../../src/string/stringOperations.c ^
    ../../src/string/stringTemplate.c ^
    ../../src/console/console.c ^
    ../../arch/console/windows-amd64/console.c ^
    ../../arch/memory/windows-amd64/memory.c ^
    -lkernel32 ^
    -o demo.exe

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    echo         If the log shows "undefined reference to __emutls_get_address",
    echo         src/trace/trace.c is using emulated TLS. Route thread-local
    echo         storage through arch/tls/ instead of _Thread_local/__thread.
    exit /b 1
)

strip --strip-unneeded demo.exe
echo Build complete: demo.exe
