@echo off
REM Build script for stringSubstring tests - Windows AMD64

setlocal enabledelayedexpansion

REM Get the directory of this script
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

REM Compiler
set CC=gcc

REM Compiler flags
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0

echo Building stringSubstring tests...
%CC% %CFLAGS% ^
    %SCRIPT_DIR%test.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    %PROJECT_ROOT%\src\console\console.c ^
    %PROJECT_ROOT%\arch\console\windows-amd64\console.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c ^
    %PROJECT_ROOT%\src\string\stringConversion.c ^
    -o %SCRIPT_DIR%test.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build complete!
exit /b 0
