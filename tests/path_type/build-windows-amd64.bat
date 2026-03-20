@echo off
REM Build script for path_type tests - Windows AMD64
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..
set CC=gcc
set CFLAGS=-I%PROJECT_ROOT%\include -Wall -Wextra -g -O0

set SOURCES=^
    %PROJECT_ROOT%\src\filesystem\path.c ^
    %PROJECT_ROOT%\arch\filesystem\windows-amd64\path.c ^
    %PROJECT_ROOT%\arch\memory\windows-amd64\memory.c ^
    %PROJECT_ROOT%\src\string\stringValidation.c ^
    %PROJECT_ROOT%\src\string\stringOperations.c

echo Building path_type tests...

%CC% %CFLAGS% %SCRIPT_DIR%test_path_conversion.c %SOURCES% -o %SCRIPT_DIR%test.exe
if %ERRORLEVEL% neq 0 ( echo Build failed! & exit /b 1 )

%CC% %CFLAGS% %SCRIPT_DIR%test_path_operations.c %SOURCES% -o %SCRIPT_DIR%test_path_operations.exe
if %ERRORLEVEL% neq 0 ( echo Build failed! & exit /b 1 )

%CC% %CFLAGS% %SCRIPT_DIR%test_path_component_access.c %SOURCES% -o %SCRIPT_DIR%test_path_component_access.exe
if %ERRORLEVEL% neq 0 ( echo Build failed! & exit /b 1 )

echo Build successful!
exit /b 0
