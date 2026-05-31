@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..

set CC=gcc
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0

set SOURCES=^
    %SCRIPT_DIR%demo.c ^
    %PROJECT_ROOT%\src\json\tokenizer.c ^
    %PROJECT_ROOT%\arch\file\windows-amd64\fileRead.c ^
    %PROJECT_ROOT%\arch\file\windows-amd64\fileReadMmap.c ^
    %PROJECT_ROOT%\arch\file\windows-amd64\fileReadRing.c ^
    %PROJECT_ROOT%\src\stream\streamFile.c ^
    %PROJECT_ROOT%\src\stream\streamLifecycle_windows.c ^
    %PROJECT_ROOT%\src\stream\streamFlow.c ^
    %PROJECT_ROOT%\arch\stream\windows-amd64\streamOpen.c ^
    %PROJECT_ROOT%\arch\stream\windows-amd64\streamRead.c ^
    %PROJECT_ROOT%\arch\stream\windows-amd64\streamWrite.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c ^
    %PROJECT_ROOT%\src\string\stringConversion.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    %PROJECT_ROOT%\src\string\stringTemplate.c ^
    %PROJECT_ROOT%\src\console\console.c ^
    %PROJECT_ROOT%\src\async\async.c ^
    %PROJECT_ROOT%\arch\console\windows-amd64\console.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c ^
    %PROJECT_ROOT%\arch\async\windows-amd64\async.c ^
    %PROJECT_ROOT%\arch\config\windows-amd64\env.c

echo Building glTF parser demo...
%CC% %CFLAGS% %SOURCES% -o %SCRIPT_DIR%demo.exe

echo Build complete: demo.exe
