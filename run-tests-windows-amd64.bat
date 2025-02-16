@echo off
setlocal enabledelayedexpansion

for /D %%d in (tests\*) do (
    cd "%%d"
    echo Compiling tests in %%d
    del .\test.exe
    call .\build-windows-amd64.bat
    if !errorlevel! neq 0 (
        echo Compilation failed in %%d
        exit /b 1
    )
    .\test.exe
    cd ..\..
)

echo All tests completed successfully
