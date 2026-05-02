@ECHO OFF
REM Validates demo directory before build
REM Usage: cd demos\<name> && ..\validate-demo.bat

setlocal enabledelayedexpansion
set ERROR_COUNT=0

echo [INFO] Validating demo directory: %CD%
echo.

REM Check demo.c exists
IF NOT EXIST demo.c (
    echo [ERROR] demo.c not found in current directory
    SET /A ERROR_COUNT+=1
)

REM Check build script exists
IF NOT EXIST build-windows-amd64.bat (
    echo [ERROR] build-windows-amd64.bat not found
    SET /A ERROR_COUNT+=1
)

REM Extract source files from build script and verify they exist
echo [INFO] Checking source file references...
FOR /F "tokens=*" %%L IN ('findstr /R "\\.c" build-windows-amd64.bat 2^>nul') DO (
    FOR %%S IN (%%L) DO (
        IF /I "%%~xS"==".c" (
            IF NOT EXIST "%%S" (
                echo [ERROR] Source file not found: %%S
                SET /A ERROR_COUNT+=1
            )
        )
    )
)

REM Check for common wrong patterns in demo.c
IF EXIST demo.c (
    findstr /C:"fun_file_read(" demo.c >nul 2>&1
    IF NOT ERRORLEVEL 1 (
        echo [ERROR] Wrong API: fun_file_read^() - use fun_read_file_in_memory^(Read^)
        echo [HINT] See include/fundamental/file/file.h
        SET /A ERROR_COUNT+=1
    )
    
    findstr /C:"src/file/file.c" demo.c >nul 2>&1
    IF NOT ERRORLEVEL 1 (
        echo [ERROR] Wrong path: src/file/file.c does not exist
        echo [HINT] File I/O is in arch/file/^<platform^>/
        SET /A ERROR_COUNT+=1
    )
    
    findstr /C:"arch/filesystem/windows-amd64/file.c" demo.c >nul 2>&1
    IF NOT ERRORLEVEL 1 (
        echo [ERROR] Wrong path: arch/filesystem/windows-amd64/file.c does not exist
        echo [HINT] Use arch/file/windows-amd64/fileRead.c
        SET /A ERROR_COUNT+=1
    )
)

REM Check headers referenced in demo.c exist
IF EXIST demo.c (
    FOR /F "tokens=2 delims=<>" %%H IN ('findstr /R "#include.*fundamental" demo.c 2^>nul') DO (
        IF NOT EXIST "..\..\include\%%H" (
            echo [ERROR] Header not found: ..\..\include\%%H
            SET /A ERROR_COUNT+=1
        )
    )
)

REM Summary
echo.
IF %ERROR_COUNT% GTR 0 (
    echo ========================================================
    echo [FAILED] %ERROR_COUNT% error^((s^) found - FIX BEFORE BUILDING
    echo ========================================================
    echo.
    echo [HINT] Read the checklist in AGENTS.md section:
    echo        "Before Writing Demo/Test Code"
    echo.
    echo [HINT] Read existing tests: tests/^<module^>/build-windows-amd64.bat
    echo.
    echo [HINT] Read headers: include/fundamental/^<module^>/^<module^>.h
    echo.
    endlocal
    exit /b 1
) ELSE (
    echo ========================================================
    echo [OK] Validation passed - ready to build
    echo ========================================================
    endlocal
    exit /b 0
)
