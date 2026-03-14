@echo off
REM Build script for platform tests - Windows AMD64

setlocal enabledelayedexpansion

REM Get the directory of this script
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

REM Compiler flags
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0

REM Source files
set SOURCES=^
    %SCRIPT_DIR%test_platform.c ^
    %PROJECT_ROOT%\src\platform\platform.c ^
    %PROJECT_ROOT%\arch\platform\windows-amd64\platform.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c

REM Build
echo Building platform tests...
gcc %CFLAGS% %SOURCES% -o %SCRIPT_DIR%test.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful!
exit /b 0
