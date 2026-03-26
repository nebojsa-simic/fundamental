@echo off
REM Build script for filesystem module tests - Windows AMD64

setlocal enabledelayedexpansion

REM Get the directory of this script
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

REM Compiler (use gcc from mingw which has its own toolchain)
set CC=C:\Users\nsimi\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\gcc.exe

REM Compiler flags
set CFLAGS=-I%PROJECT_ROOT%\include -I%PROJECT_ROOT%\arch\filesystem\windows-amd64 -Wall -Wextra -g -O0

REM Source files
set SOURCES=^
    %SCRIPT_DIR%test_filesystem.c ^
    %PROJECT_ROOT%\src\filesystem\directory.c ^
    %PROJECT_ROOT%\src\filesystem\file_exists.c ^
    %PROJECT_ROOT%\src\filesystem\file_size.c ^
    %PROJECT_ROOT%\src\filesystem\path.c ^
    %PROJECT_ROOT%\src\filesystem\walk.c ^
    %PROJECT_ROOT%\src\tsv\tsv.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\directory.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\file_exists.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\file_size.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\path.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c

REM Build
echo Building filesystem tests...
%CC% %CFLAGS% %SOURCES% -o %SCRIPT_DIR%test.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful!
exit /b 0
