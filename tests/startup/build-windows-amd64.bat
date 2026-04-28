@echo off
REM Build script for startup tests - Windows AMD64

setlocal enabledelayedexpansion

REM Get the directory of this script
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

REM Compiler flags
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0 -DFUNDAMENTAL_STARTUP_VERBOSE=1

REM Source files
set SOURCES=^
    %SCRIPT_DIR%test.c ^
    %PROJECT_ROOT%\src\startup\startup.c ^
    %PROJECT_ROOT%\arch\startup\windows-amd64\windows.c ^
    %PROJECT_ROOT%\src\platform\platform.c ^
    %PROJECT_ROOT%\arch\platform\windows-amd64\platform.c ^
    %PROJECT_ROOT%\src\filesystem\path.c ^
    %PROJECT_ROOT%\src\filesystem\file_exists.c ^
    %PROJECT_ROOT%\src\filesystem\file_size.c ^
    %PROJECT_ROOT%\src\filesystem\directory.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\path.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\file_exists.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\file_size.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\directory.c ^
    %PROJECT_ROOT%\src\config\config.c ^
    %PROJECT_ROOT%\src\config\iniParser.c ^
    %PROJECT_ROOT%\src\config\cliParser.c ^
    %PROJECT_ROOT%\arch\config\windows-amd64\env.c ^
    %PROJECT_ROOT%\src\network\network.c ^
    %PROJECT_ROOT%\arch\network\windows-amd64\network.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringConversion.c ^
    %PROJECT_ROOT%\src\string\stringTemplate.c ^
    %PROJECT_ROOT%\src\hashmap\hashmap.c ^
    %PROJECT_ROOT%\src\console\console.c ^
    %PROJECT_ROOT%\arch\console\windows-amd64\console.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c

REM Build
echo Building startup tests...
gcc %CFLAGS% %SOURCES% -lws2_32 -o %SCRIPT_DIR%test.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful!
exit /b 0
