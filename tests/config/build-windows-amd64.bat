@echo off
REM Build script for config module tests - Windows AMD64

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
    %SCRIPT_DIR%test_config.c ^
    %PROJECT_ROOT%\src\config\config.c ^
    %PROJECT_ROOT%\src\config\iniParser.c ^
    %PROJECT_ROOT%\src\config\cliParser.c ^
    %PROJECT_ROOT%\arch\config\windows-amd64\env.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c ^
    %PROJECT_ROOT%\src\hashmap\hashmap.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    %PROJECT_ROOT%\src\filesystem\path.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\path.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\file_exists.c ^
    %PROJECT_ROOT%\src\filesystem\file_exists.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\directory.c

REM Build
echo Building config tests...
%CC% %CFLAGS% %SOURCES% -o %SCRIPT_DIR%test.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful!
exit /b 0
