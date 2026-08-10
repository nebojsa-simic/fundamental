@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

set CC=gcc
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0

set SOURCES=^
    %SCRIPT_DIR%test_json.c ^
    %PROJECT_ROOT%\src\json\tokenizer.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\console\console.c ^
    %PROJECT_ROOT%\arch\console\windows-amd64\console.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c ^
    %PROJECT_ROOT%\src\string\stringConversion.c

echo Building json tests...
%CC% %CFLAGS% %SOURCES% -o %SCRIPT_DIR%test.exe

echo Build complete: test.exe
