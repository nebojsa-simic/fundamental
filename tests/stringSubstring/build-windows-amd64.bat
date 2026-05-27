@echo off
REM Build script for stringSubstring tests - Windows AMD64

setlocal enabledelayedexpansion

REM Get the directory of this script
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

REM Compiler
set CC=C:\Users\nsimi\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\gcc.exe

REM Compiler flags
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0

echo Building stringSubstring tests...
%CC% %CFLAGS% ^
    %SCRIPT_DIR%test.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    -o %SCRIPT_DIR%test.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build complete!
exit /b 0
