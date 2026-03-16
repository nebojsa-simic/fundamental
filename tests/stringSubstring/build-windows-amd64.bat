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

REM Build test_substring.exe
echo Building test_substring.exe...
%CC% %CFLAGS% ^
    %SCRIPT_DIR%test_substring.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    -o %SCRIPT_DIR%test_substring.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed for test_substring.exe
    exit /b 1
)

REM Build test_slice.exe
echo Building test_slice.exe...
%CC% %CFLAGS% ^
    %SCRIPT_DIR%test_slice.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    -o %SCRIPT_DIR%test_slice.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed for test_slice.exe
    exit /b 1
)

echo Build complete!
exit /b 0
