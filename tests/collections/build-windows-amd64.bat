@echo off
set CC=gcc
set CFLAGS=-I../../include -std=c11 -Wall -Wextra -g
set SOURCES=test.c
set OUTPUT=test.exe
set LIBS=-lkernel32 -ladvapi32

REM We're referencing array functionality that doesn't exist yet - this will fail until implemented
REM This build script is a placeholder until array module is implemented

REM First check if include array header exists
if not exist "..\..\include\array" (
    echo WARNING: Array module not implemented yet. 
    echo Create include/array/array.h with functions like:
    echo   - fun_array_int_create
    echo   - fun_array_int_push  
    echo   - fun_array_int_get
    echo   - fun_array_int_destroy
    echo Before running this build script.
    exit /b 1
)

%CC% %CFLAGS% %SOURCES% %LIBS% -o %OUTPUT%