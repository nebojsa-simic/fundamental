@echo off
REM Build script for path_type tests on Windows AMD64
setlocal enabledelayedexpansion

echo Building path_type tests...

REM Set up paths
set "SRC_DIR=..\..\src"
set "INCLUDE_DIR=..\..\include"
set "ARCH_DIR=..\..\arch"

REM Compile test_path_conversion.c
echo Compiling test_path_conversion.c...
cl /nologo /W4 /I"%INCLUDE_DIR%" /I"%ARCH_DIR%\windows-amd64" ^
   test_path_conversion.c ^
   "%SRC_DIR%\filesystem\path.c" ^
   "%SRC_DIR%\filesystem\directory.c" ^
   "%SRC_DIR%\memory\memory.c" ^
   "%SRC_DIR%\string\string.c" ^
   "%SRC_DIR%\error\error.c" ^
   "%ARCH_DIR%\windows-amd64\directory.c" ^
   "%ARCH_DIR%\windows-amd64\path.c" ^
   /Fe:test_conversion.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed for test_path_conversion.c
    exit /b 1
)

REM Compile test_path_operations.c
echo Compiling test_path_operations.c...
cl /nologo /W4 /I"%INCLUDE_DIR%" /I"%ARCH_DIR%\windows-amd64" ^
   test_path_operations.c ^
   "%SRC_DIR%\filesystem\path.c" ^
   "%SRC_DIR%\filesystem\directory.c" ^
   "%SRC_DIR%\memory\memory.c" ^
   "%SRC_DIR%\string\string.c" ^
   "%SRC_DIR%\error\error.c" ^
   "%ARCH_DIR%\windows-amd64\directory.c" ^
   "%ARCH_DIR%\windows-amd64\path.c" ^
   /Fe:test_operations.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed for test_path_operations.c
    exit /b 1
)

REM Compile test_path_component_access.c
echo Compiling test_path_component_access.c...
cl /nologo /W4 /I"%INCLUDE_DIR%" /I"%ARCH_DIR%\windows-amd64" ^
   test_path_component_access.c ^
   "%SRC_DIR%\filesystem\path.c" ^
   "%SRC_DIR%\filesystem\directory.c" ^
   "%SRC_DIR%\memory\memory.c" ^
   "%SRC_DIR%\string\string.c" ^
   "%SRC_DIR%\error\error.c" ^
   "%ARCH_DIR%\windows-amd64\directory.c" ^
   "%ARCH_DIR%\windows-amd64\path.c" ^
   /Fe:test_component_access.exe

if %ERRORLEVEL% neq 0 (
    echo Build failed for test_path_component_access.c
    exit /b 1
)

echo Build complete!
exit /b 0
