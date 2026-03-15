@echo off
REM Build script for network module tests - Windows AMD64

setlocal enabledelayedexpansion

REM Get the directory of this script
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

REM Compiler (use gcc from mingw which has its own toolchain)
set CC=C:\Users\nsimi\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\gcc.exe

REM Compiler flags
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0

REM Source files
set SOURCES=^
    %SCRIPT_DIR%test_network.c ^
    %PROJECT_ROOT%\src\network\network.c ^
    %PROJECT_ROOT%\arch\network\windows-amd64\network.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c

REM Build
echo Building network tests...
%CC% %CFLAGS% %SOURCES% -o %SCRIPT_DIR%test.exe -lws2_32 -lmswsock

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful!
exit /b 0
